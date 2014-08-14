/*****************************************************************************/
/*         (c) Copyright, Real-Time Innovations, All rights reserved.        */
/*                                                                           */
/* Permission to modify and use for internal purposes granted.               */
/* This software is provided "as is", without warranty, express or implied.  */
/*                                                                           */
/*****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>


#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>
#include <gst/video/videooverlay.h>

#include "ndds/ndds_cpp.h"

#include "VideoData.h"
#include "VideoDataSupport.h"
#include "VideoDataPlugin.h"

using namespace com::rti::media::generated;

static JavaVM *gJvm;

static DDSDomainParticipant *gParticipant = NULL;
static VideoStreamDataWriter *gWriter = NULL;
static VideoStream *gStreamData = NULL;

static GstPipeline *gPipeline = NULL;
static GstAppSrc   *gAppSrc = NULL;
static GstAppSink  *gAppSink = NULL;
static GstElement  *gVideoSink = NULL;
static ANativeWindow *gWindow = NULL;

static long current_stream_id = -1;
static bool paused = true;

/*************************************************************************
 */
void Android_Log_i(const char *format, ...)
{
	va_list arglist;
	va_start(arglist, format);

	__android_log_vprint(ANDROID_LOG_INFO, "video-connext-gstreamer", format, arglist);
	va_end(arglist);
}

/*************************************************************************
 */
void Android_Log_e(const char *format, ...)
{
	va_list arglist;
	va_start(arglist, format);

	__android_log_vprint(ANDROID_LOG_ERROR, "video-connext-gstreamer", format,
			arglist);
	va_end(arglist);
}

extern "C" {


/*************************************************************************
 * Pauses the gstreamer pipeline
 */
JNIEXPORT void JNICALL Java_com_rti_android_videodemo_VideoStreamConnext_ConnextGstreamer_1pause(
		JNIEnv* env, jobject self)
{
	if ( gPipeline != NULL ) {
		gst_element_set_state (GST_ELEMENT(gPipeline), GST_STATE_PAUSED);
		paused = true;
		Android_Log_i("Pipeline paused.");
	}

	// pausing display will allow display to lock on to different video streams
	current_stream_id = -1;
}

/*************************************************************************
 * Starts the gsteamer pipeline processing
 */
JNIEXPORT void JNICALL Java_com_rti_android_videodemo_VideoStreamConnext_ConnextGstreamer_1play(
		JNIEnv* env, jobject self)
{
	if ( gPipeline != NULL ) {
		gst_element_set_state (GST_ELEMENT(gPipeline), GST_STATE_PLAYING);
		paused = false;
		Android_Log_i("Pipeline playing.");
	}
}


/*************************************************************************
 * Called when a video frame is receive from DDS
 */
class FrameListener : public DDSDataReaderListener
{
	virtual void on_data_available( DDSDataReader *reader )
	{
		VideoStreamDataReader *videoReader = VideoStreamDataReader::narrow(reader);
		VideoStreamSeq dataSeq;
		DDS_SampleInfoSeq infoSeq;
		DDS_ReturnCode_t retCode = DDS_RETCODE_OK;

		if (videoReader == NULL)
		{
			Android_Log_e("FrameListener: Bad reader detected");
			return;
		}

		retCode = videoReader->take(dataSeq, infoSeq);

		if ( (retCode != DDS_RETCODE_OK) &&
				(retCode != DDS_RETCODE_NO_DATA) )
		{
			Android_Log_e("Error receiving data");
			return;
		}

		for (int i = 0; i < dataSeq.length(); i++)
		{
			if (infoSeq[i].valid_data == DDS_BOOLEAN_TRUE)
			{
				GstBuffer *buffer = NULL;
				GstMemory *memory = NULL;

				if (paused) {
					continue;
				}

				// make sure processing frame from same stream
				if (current_stream_id == -1) {
					current_stream_id = dataSeq[i].stream_id;
				}
				else if (dataSeq[i].stream_id != dataSeq[i].stream_id) {
					// got a frame from a different stream, drop/ignore
					continue;
				}

				char* bytes = (char *)dataSeq[i].frame.get_contiguous_buffer();
				int  length = dataSeq[i].frame.length();

				// Allocate a new buffer from the GStreamer framework that will be
				// used to display this video frame.
				buffer = gst_buffer_new();
				memory = gst_allocator_alloc(NULL, length, NULL);
				gst_buffer_insert_memory(buffer, -1, memory);

				if ( length != gst_buffer_fill(buffer, 0, bytes, length) ) {
					Android_Log_e("ConnextGstreamer failed to copy %d bytes", length);
					gst_buffer_remove_all_memory(buffer);
					delete(buffer);
				} else {

					// The buffer becomes managed by the GStreamer framework as soon as
					// we push it, so we do not have to free it.
					if ( gst_app_src_push_buffer(gAppSrc, buffer) != GST_FLOW_OK ) {
						Android_Log_e("ConnextGstreamer failed to push buffer.");
						gst_buffer_remove_all_memory(buffer);
						delete(buffer);
					}
					//Android_Log_i("Received frame via DDS of length %d", length);
				}
			}
		}

		videoReader->return_loan(dataSeq, infoSeq);

		return;
	};
};


/*************************************************************************
 * Function that sends the encoded frame via DDS
 */
static void SendFrameDDS(DDS_Octet *frame, int length)
{
	DDS_ReturnCode_t retcode;

	if ( gWriter == NULL ) {
		Android_Log_e("Failed to send frame, writer not created yet.");
		return;
	}

	++gStreamData->sequence_number;

	// no copy, just use the buffer of bytes
	gStreamData->frame.loan_contiguous(frame, length, length);

	retcode = gWriter->write(*gStreamData, DDS_HANDLE_NIL);

	gStreamData->frame.unloan();

	if (retcode != DDS_RETCODE_OK) {
		Android_Log_e("Failed to send frame %d", retcode);
	} else {
	    //Android_Log_i("Sent frame via DDS of length %d", length);
	}
}


/*************************************************************************
 * Called when there is a new encoded frame in the gstreamer pipe
 */
static GstFlowReturn NewFrame(GstAppSink *appsink, gpointer user_data)
{
	DDS_Octet *frame;
	int length;

	GstSample *sample = gst_app_sink_pull_sample(appsink);
	if (sample == NULL) {
		Android_Log_e("NewFrame called with no sample");
		return GST_FLOW_ERROR;
	}

	GstBuffer* buffer = gst_sample_get_buffer(sample);
	if (buffer == NULL) {
		Android_Log_e("NewFrame called with no buffer");
		gst_sample_unref(sample);
		return GST_FLOW_ERROR;
	}

	GstMapInfo map;
	if (!gst_buffer_map (buffer, &map, GST_MAP_READ)) {
		Android_Log_e("NewFrame could not map buffer");
		gst_sample_unref(sample);
		return GST_FLOW_ERROR;
	}

	length = map.size;
	frame  = map.data;

	if (length > MAX_BUFFER_SIZE) {
		Android_Log_e("NewFrame buffer too large %d", length);
		gst_buffer_unmap(buffer, &map);
		gst_sample_unref(sample);
		return GST_FLOW_ERROR;
	}

	// Send data via DDS
	SendFrameDDS(frame,length);

	gst_buffer_unmap(buffer, &map);
	gst_sample_unref(sample);

	return GST_FLOW_OK;
}


/*************************************************************************
 * Called from Java with raw video frame
 */
JNIEXPORT jboolean JNICALL Java_com_rti_android_videodemo_VideoStreamConnext_ConnextGstreamer_1sendFrame(
		JNIEnv* env, jobject self, jbyteArray frame)
{
	GstBuffer *buffer = NULL;
	GstMemory *memory = NULL;
	jboolean retCode = JNI_FALSE;

	jbyte* bytes  = env->GetByteArrayElements(frame, NULL);
	jsize  length = env->GetArrayLength(frame);

	// Allocate a new buffer from the GStreamer framework that will be
	// used to display this video frame.
	buffer = gst_buffer_new();

	memory = gst_allocator_alloc(NULL, length, NULL);
	gst_buffer_insert_memory(buffer, -1, memory);

	if ( length != gst_buffer_fill(buffer,0,bytes,length) ) {
		Android_Log_e("ConnextGstreamer failed to copy %d bytes", length);
		goto done;
	}

	// The buffer becomes managed by the GStreamer framework as soon as
	// we push it, so we do not have to free it.
	if ( gst_app_src_push_buffer(gAppSrc, buffer) != GST_FLOW_OK ) {
		Android_Log_e("ConnextGstreamer failed to push buffer.");
		goto done;
	}

	retCode = JNI_TRUE;

	done:
	env->ReleaseByteArrayElements(frame, bytes, JNI_ABORT);
	return retCode;
}


/*************************************************************************
 * Need to call this function once to initialize publishing video.
 */
JNIEXPORT jboolean JNICALL Java_com_rti_android_videodemo_VideoStreamConnext_ConnextGstreamer_1initializePub(
		JNIEnv* env, jobject self, jint domainID, jstring jpeers, jint width, jint height)
{

	DDS_Boolean success = DDS_BOOLEAN_FALSE;
	char* peerlist = NULL;

	{ // DDS Initialization
		DDS_DomainParticipantQos dp_qos;
		DDSPublisher *publisher = NULL;
		DDSTopic *topic = NULL;
		DDSDataWriter *writer = NULL;

		DDS_ReturnCode_t retcode;
		const char *type_name = NULL;

		Android_Log_i("Creating participant...");

		DDSTheParticipantFactory->get_default_participant_qos(dp_qos);

		/* Have to get the c-string from Java */
		peerlist = (char *) env->GetStringUTFChars(jpeers, NULL);

		/* Use user specified peer list if set */
		if ( (peerlist != NULL ) && (strlen(peerlist) != 0) ) {
			if ( strchr(peerlist, ',') )
			{
				int cnt = 1;
				int i = 0;
				char* pch;

				/* Get the number of peers */
				while (i < strlen(peerlist))
				{
					if (peerlist[i] == ',')
					{
						cnt ++;
					}
					i++;
				}

				dp_qos.discovery.initial_peers.ensure_length(cnt,cnt);

				pch = strtok(peerlist, ",");

				i = 0;
				while ((pch != NULL) && (i < cnt))
				{
					dp_qos.discovery.initial_peers[i] = DDS_String_dup(pch);
					i++;
					pch = strtok(NULL,",");
				}
			}
			else
			{
				dp_qos.discovery.initial_peers.ensure_length(1,1);
				dp_qos.discovery.initial_peers[0] = DDS_String_dup(peerlist);
			}
			Android_Log_i("Using custom peer list : %s", peerlist);

			/* Should release the string, else memory leak */
			env->ReleaseStringUTFChars(jpeers, peerlist);
		}

		gParticipant = DDSTheParticipantFactory->create_participant(
				domainID, dp_qos, NULL , DDS_STATUS_MASK_NONE);

		if ( gParticipant == NULL )
		{
			Android_Log_e("Failed to create participant.");
			goto done;
		}


		type_name = VideoStreamTypeSupport::get_type_name();
		retcode = VideoStreamTypeSupport::register_type(
				gParticipant, type_name);
		if ( retcode != DDS_RETCODE_OK ) {
			Android_Log_e("Failed to register type %d\n", retcode);
			goto done;
		}

		publisher = gParticipant->create_publisher(
				DDS_PUBLISHER_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);
		if ( publisher == NULL ) {
			Android_Log_e("Failed to create publisher.");
			goto done;
		}

		topic = gParticipant->create_topic(
				VIDEO_TOPIC, type_name, DDS_TOPIC_QOS_DEFAULT, NULL,
				DDS_STATUS_MASK_NONE);
		if ( topic == NULL ) {
			Android_Log_e("Failed to create topic.");
			goto done;
		}

		writer = publisher->create_datawriter(
				topic, DDS_DATAWRITER_QOS_DEFAULT, NULL,
				DDS_STATUS_MASK_NONE);
		if ( writer == NULL ) {
			Android_Log_e("Failed to create writer.");
			goto done;
		}

		gWriter = VideoStreamDataWriter::narrow(writer);
		if ( gWriter == NULL ) {
			Android_Log_e("Failed to narrow writer.");
			goto done;
		}

		gStreamData = VideoStreamTypeSupport::create_data();

		if ( gStreamData == NULL ) {
			Android_Log_e("Failed to create VideoStream data.");
			goto done;
		}

		// Hardcoding id, if want multiple video streams, need to have
		// unique id for each stream
		gStreamData->stream_id = (int)env;

		// Will be loaning buffer, so clear internally allocated memory
		gStreamData->frame.maximum(0);

	}

	Android_Log_i("RTI Connext initialized.");

	{ // Gstreamer must already be initialized via Java call

      // Debug output will go to logcat...only works if using gtreamer debug runtime
	    //gst_debug_set_default_threshold(GST_LEVEL_DEBUG);

		char pipeline[1024];
		GError *error = NULL;

		// NOTE:  videoscale does not handle NV21 format
	    // Need the following plugins
	    // coreelements videoconvert videoscale app autodetect vpx opensles opengl
	    // GSTREAMER_EXTRA_DEPS   := gstreamer-video-1.0

		sprintf(pipeline, "appsrc name=\"src\" is-live=\"true\" do-timestamp=\"true\" "
				" min-latency=0 max-latency=100000000 "
				" ! video/x-raw, format=NV21, width=%d, height=%d, framerate=(fraction)15/1 "
				" ! videoconvert "
				" ! vp8enc deadline=1 threads=5 keyframe-max-dist=30 buffer-size=10 buffer-initial-size=10 buffer-optimal-size=10 "
				" ! video/x-vp8, profile=(string)2 "
				" ! appsink name=\"sink\" ",
				width, height);
		gPipeline =
				(GstPipeline *)gst_parse_launch(pipeline, &error);

		if (error) {
			Android_Log_e("Failed to create gstreamer pipeline: %s.", error->message);
			g_clear_error (&error);
			return JNI_FALSE;
		}

		// Setup appsrc as a stream
		gAppSrc  = (GstAppSrc*)  gst_bin_get_by_name((GstBin *)gPipeline, "src");
		gst_app_src_set_stream_type(gAppSrc, GST_APP_STREAM_TYPE_STREAM);
		g_object_set(gAppSrc, "format", GST_FORMAT_TIME, NULL);

		gAppSink = (GstAppSink*) gst_bin_get_by_name((GstBin *)gPipeline, "sink");

		// Setup callback to get vp8 encoded stream
		GstAppSinkCallbacks callbacks;

		callbacks.eos = NULL;
		callbacks.new_preroll = NULL;
		callbacks.new_sample = NewFrame;

		gst_app_sink_set_callbacks (gAppSink, &callbacks,
				NULL, NULL);

		gst_element_set_state((GstElement*)gPipeline, GST_STATE_PLAYING);
	}

	success = DDS_BOOLEAN_TRUE;
	Android_Log_i("ConnextGstreamer video camera initialized width=%d height=%d.", width, height);

	done:

	if (!success)
	{
		if ( gParticipant != NULL )
		{
			gParticipant->delete_contained_entities();
			DDSTheParticipantFactory->delete_participant(gParticipant);
		}
		gParticipant = NULL;
		return JNI_FALSE;
	}

	return JNI_TRUE;
}


/*************************************************************************
 */
JNIEXPORT jboolean JNICALL Java_com_rti_android_videodemo_VideoStreamConnext_ConnextGstreamer_1finalizePub(
		JNIEnv* env, jobject self)
{
	DDS_ReturnCode_t retcode;

	// stop gstreamer
	gst_element_set_state((GstElement*)gPipeline, GST_STATE_NULL);
	gst_object_unref(gAppSrc);
	gst_object_unref(gAppSink);
	gst_object_unref(gPipeline);

	gAppSrc = NULL;
	gAppSink = NULL;
	gPipeline = NULL;

	if ( gParticipant == NULL) {
		return JNI_TRUE;
	}
	/* first delete all DDS child-entities of the participant */
	retcode = gParticipant->delete_contained_entities();

	if ( retcode != DDS_RETCODE_OK )
	{
		Android_Log_e("Failed to delete contained entities: %d", retcode);
		return JNI_FALSE;
	}

	/* Now can delete participant */
	retcode = DDSTheParticipantFactory->delete_participant(gParticipant);

	if ( retcode != DDS_RETCODE_OK )
	{
		Android_Log_e("Failed to delete DomainParticipant: %d", retcode);
		return JNI_FALSE;
	}

	gParticipant = NULL;

	if ( gStreamData != NULL ) {
		VideoStreamTypeSupport::delete_data(gStreamData);
	}
	gStreamData = NULL;

	Android_Log_i("ConnextGstreamer finalized.");

	return JNI_TRUE;
}



/*************************************************************************
 * Need to call this function once to initialize publishing video.
 */
JNIEXPORT jboolean JNICALL Java_com_rti_android_videodemo_VideoStreamConnext_ConnextGstreamer_1initializeSub(
		JNIEnv* env, jobject self, jint domainID, jstring jpeers, jboolean use_multicast)
{

	DDS_Boolean success = DDS_BOOLEAN_FALSE;
	char* peerlist = NULL;

	// DDS Initialization
	DDS_DomainParticipantQos dp_qos;
	DDSSubscriber *subscriber = NULL;
	DDSTopic *topic = NULL;
	DDSDataReader *reader = NULL;
	DDSDataReaderListener *listener = new FrameListener();
	{
		DDS_ReturnCode_t retcode;
		const char *type_name = NULL;

		Android_Log_i("Creating participant...");

		DDSTheParticipantFactory->get_default_participant_qos(dp_qos);

		/* Have to get the c-string from Java */
		peerlist = (char *) env->GetStringUTFChars(jpeers, NULL);

		/* Use user specified peer list if set */
		if ( (peerlist != NULL ) && (strlen(peerlist) != 0) ) {
			if ( strchr(peerlist, ',') )
			{
				int cnt = 1;
				int i = 0;
				char* pch;

				/* Get the number of peers */
				while (i < strlen(peerlist))
				{
					if (peerlist[i] == ',')
					{
						cnt ++;
					}
					i++;
				}

				dp_qos.discovery.initial_peers.ensure_length(cnt,cnt);

				pch = strtok(peerlist, ",");

				i = 0;
				while ((pch != NULL) && (i < cnt))
				{
					dp_qos.discovery.initial_peers[i] = DDS_String_dup(pch);
					i++;
					pch = strtok(NULL,",");
				}
			}
			else
			{
				dp_qos.discovery.initial_peers.ensure_length(1,1);
				dp_qos.discovery.initial_peers[0] = DDS_String_dup(peerlist);
			}
			Android_Log_i("Using custom peer list : %s", peerlist);

			/* Should release the string, else memory leak */
			env->ReleaseStringUTFChars(jpeers, peerlist);
		}

		gParticipant = DDSTheParticipantFactory->create_participant(
				domainID, dp_qos, NULL , DDS_STATUS_MASK_NONE);

		if ( gParticipant == NULL )
		{
			Android_Log_e("Failed to create participant.");
			goto done;
		}


		type_name = VideoStreamTypeSupport::get_type_name();
		retcode = VideoStreamTypeSupport::register_type(
				gParticipant, type_name);
		if ( retcode != DDS_RETCODE_OK ) {
			Android_Log_e("Failed to register type %d\n", retcode);
			goto done;
		}

		subscriber = gParticipant->create_subscriber(
				DDS_SUBSCRIBER_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);
		if ( subscriber == NULL ) {
			Android_Log_e("Failed to create subscriber.");
			goto done;
		}

		topic = gParticipant->create_topic(
				VIDEO_TOPIC, type_name, DDS_TOPIC_QOS_DEFAULT, NULL,
				DDS_STATUS_MASK_NONE);
		if ( topic == NULL ) {
			Android_Log_e("Failed to create topic.");
			goto done;
		}

		DDS_DataReaderQos reader_qos;
		subscriber->get_default_datareader_qos(reader_qos);

		if (use_multicast) {
			reader_qos.multicast.value.ensure_length(1,1);
			reader_qos.multicast.value[0].receive_address = DDS_String_dup("224.0.0.1");
		}

		reader = subscriber->create_datareader(
				topic, reader_qos, listener,
				DDS_DATA_AVAILABLE_STATUS);
		if ( reader == NULL ) {
			Android_Log_e("Failed to create reader.");
			goto done;
		}
	}
	Android_Log_i("RTI Connext initialized.");

	success = DDS_BOOLEAN_TRUE;

	done:

	if (!success)
	{

		if ( gParticipant != NULL )
		{
			gParticipant->delete_contained_entities();
			DDSTheParticipantFactory->delete_participant(gParticipant);
		}
		gParticipant = NULL;
		delete listener;
		return JNI_FALSE;
	}

	return JNI_TRUE;
}


/*************************************************************************
 */
JNIEXPORT jboolean JNICALL Java_com_rti_android_videodemo_VideoStreamConnext_ConnextGstreamer_1finalizeSub(
		JNIEnv* env, jobject self)
{
	DDS_ReturnCode_t retcode;

	if (gParticipant == NULL) {
		return JNI_TRUE;
	}
	/* first delete all DDS child-entities of the participant */
	retcode = gParticipant->delete_contained_entities();

	if ( retcode != DDS_RETCODE_OK )
	{
		Android_Log_e("Failed to delete contained entities: %d", retcode);
		return JNI_FALSE;
	}

	/* Now can delete participant */
	retcode = DDSTheParticipantFactory->delete_participant(gParticipant);

	if ( retcode != DDS_RETCODE_OK )
	{
		Android_Log_e("Failed to delete DomainParticipant: %d", retcode);
		return JNI_FALSE;
	}

	gParticipant = NULL;

	Android_Log_i("ConnextGstreamer finalized.");

	return JNI_TRUE;
}


/*************************************************************************
 */
JNIEXPORT void JNICALL Java_com_rti_android_videodemo_VideoStreamConnext_ConnextGstreamer_1initializeDisplay(
		JNIEnv* env, jobject self, jint width, jint height)
{
	// Gstreamer must already be initialized via Java call

    // Debug output will go to logcat...only works if using gtreamer debug runtime
	// gst_debug_set_default_threshold(GST_LEVEL_DEBUG);

	char pipeline[1024];
    GError *error = NULL;

    // Need the following plugins
    // coreelements videoconvert videoscale app autodetect vpx opensles opengl
    // GSTREAMER_EXTRA_DEPS   := gstreamer-video-1.0

	sprintf(pipeline, "appsrc name=\"src\" is-live=\"true\" do-timestamp=\"true\" "
			" min-latency=0 max-latency=100000000 "
			" ! video/x-vp8 "
			" ! vp8dec threads=5 "
			" ! autovideosink name=\"sink\" ");

    gPipeline =
        (GstPipeline *)gst_parse_launch(pipeline, &error);

	if (error) {
		Android_Log_e("Failed to create gstreamer pipeline: %s.", error->message);
		g_clear_error (&error);
		return;
	}

	// Setup appsrc as a stream
	gAppSrc  = (GstAppSrc*)  gst_bin_get_by_name((GstBin *)gPipeline, "src");

	if ( gAppSrc == NULL ) {
		Android_Log_e("Failed to get app src from gstreamer.");
        return;
	}

	gst_app_src_set_stream_type(gAppSrc, GST_APP_STREAM_TYPE_STREAM);
	g_object_set(gAppSrc, "format", GST_FORMAT_TIME, NULL);

    /* Set the pipeline to READY, so it can already accept a window handle, if we have one */
    gst_element_set_state(GST_ELEMENT(gPipeline), GST_STATE_READY);

	gVideoSink =  gst_bin_get_by_interface(GST_BIN(gPipeline), GST_TYPE_VIDEO_OVERLAY);

	if ( gVideoSink == NULL ) {
		Android_Log_e("Failed to get video sink from gstreamer.");
		return;
	}

	gst_element_set_state((GstElement*)gPipeline, GST_STATE_PLAYING);

	// will lock onto first video stream it sees
	current_stream_id = -1;

	Android_Log_i("ConnextGstreamer video display initialized");
}


/*************************************************************************
 */
JNIEXPORT void JNICALL Java_com_rti_android_videodemo_VideoStreamConnext_ConnextGstreamer_1finalizeDisplay(
		JNIEnv* env, jobject self)
{
	// stop gstreamer
	gst_element_set_state((GstElement*)gPipeline, GST_STATE_NULL);
	gst_object_unref(gAppSrc);
	gst_object_unref(gAppSink);
	gst_object_unref(gPipeline);

	gPipeline = NULL;
	gAppSink = NULL;
	gAppSrc = NULL;
}


/*************************************************************************
 */
JNIEXPORT void JNICALL Java_com_rti_android_videodemo_VideoStreamConnext_ConnextGstreamer_1setDisplayWindow(
		JNIEnv *env, jobject self, jobject surface)
{
	ANativeWindow *new_native_window = ANativeWindow_fromSurface(env, surface);

	if ( gWindow != NULL) {

		// release the reference, either we are going to replace with new one, or
		// we have to release the reference from the current one taken by ANativeWindow_fromSurface
		ANativeWindow_release ( gWindow );

		if ( gWindow == new_native_window ) {
			// Need to refresh screen?
			if ( gVideoSink ) {
				gst_video_overlay_expose(GST_VIDEO_OVERLAY(gVideoSink));
				gst_video_overlay_expose(GST_VIDEO_OVERLAY(gVideoSink));
			}

			return;
		}
	}

	gWindow = new_native_window;
	gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(gVideoSink), (guintptr)gWindow);
}


/*************************************************************************
 */
JNIEXPORT void JNICALL Java_com_rti_android_videodemo_VideoStreamConnext_ConnextGstreamer_1unsetDisplayWindow(
		JNIEnv *env, jobject self)
{
	if ( gVideoSink ) {
		gst_video_overlay_set_window_handle (GST_VIDEO_OVERLAY(gVideoSink), (guintptr)NULL);
		gst_element_set_state(GST_ELEMENT(gPipeline), GST_STATE_READY);
	}

	ANativeWindow_release(gWindow);
	gWindow = NULL;
}


/*************************************************************************
 */
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved)
{
	// Need to cache a pointer to the VM
	gJvm = jvm;

	return JNI_VERSION_1_6;
}

} // extern "C"
