/*****************************************************************************/
/*         (c) Copyright, Real-Time Innovations, All rights reserved.        */
/*                                                                           */
/*         Permission to modify and use for internal purposes granted.       */
/* This software is provided "as is", without warranty, express or implied.  */
/*                                                                           */
/*****************************************************************************/

package com.rti.android.videodemo;


import org.freedesktop.gstreamer.*;

import android.content.Context;
import android.util.Log;
import android.view.Surface;

/* Class that manages how RTI Connext DDS is used for the VideoDemo */
public class VideoStreamConnext
{
	
    public enum VideoStreamConnextType{ NONE, PUBLISHER, SUBCRIBER }
    
	private static VideoStreamConnextType isInitialized = VideoStreamConnextType.NONE;
	
	public static boolean initialize(VideoStreamConnextType type, int domainID, String peerList, Context context, int width, int height, boolean use_multicast)
	{
		if ( isInitialized == type ) {
			return true;
		}
		
        // Initialize GStreamer and warn if it fails
        try {
            GStreamer.init(context);
        } catch (Exception e) {
            Log.e("VideoStreamConnext", "Could not initialize Gstreamer" + e.getMessage());
            return false;
        }
        
        if ( type == VideoStreamConnextType.PUBLISHER ) {
			/* Here we initialize the wrapper, which needs to be done only once */
			if ( !ConnextGstreamer_initializePub(domainID, peerList, width, height) )
			{
				Log.e("VideoStreamConnext", "Could not initialize RTI Connext/Gstreamer publish");
				return false;
			}
        } else {
        
			if ( !ConnextGstreamer_initializeSub(domainID, peerList, use_multicast) )
			{
				Log.e("VideoStreamConnext", "Could not initialize RTI Connext/Gstreamer subscribe");
				return false;
			}
        }
        
        isInitialized = type;
		
		return true;
	}

	public static boolean stop()
	{
		VideoStreamConnextType type = isInitialized;
		isInitialized = VideoStreamConnextType.NONE;
		
		if ( type == VideoStreamConnextType.PUBLISHER ) {
			return ConnextGstreamer_finalizePub();
		}
		else {
			return ConnextGstreamer_finalizeSub();
		}
	}

	public static boolean write(byte[] frame)
	{
		if ( isInitialized == VideoStreamConnextType.PUBLISHER  ) {
			return ConnextGstreamer_sendFrame(frame);
		} else {
			return false;
		}
	}
	
	public static void setDisplaySurface(Surface surface) 
	{
		 ConnextGstreamer_setDisplayWindow(surface);
	}
	
	public static void unsetDisplaySurface() 
	{
		 ConnextGstreamer_unsetDisplayWindow();
	}
	
	public static void play() 
	{
		 ConnextGstreamer_play();
	}
	
	public static void pause() 
	{
		 ConnextGstreamer_pause();
	}
	
	public static void initializeDisplay( int width, int height )
	{
		ConnextGstreamer_initializeDisplay(width, height);
	}
	
	public static void finalizeDisplay() 
	{
		ConnextGstreamer_finalizeDisplay();
	}
	
	/* Declarations needed to use JNI to call native functions */
	public native static boolean ConnextGstreamer_initializePub(int domainID,
	        String peerList, int width, int height);
	public native static boolean ConnextGstreamer_finalizePub();
	public native static boolean ConnextGstreamer_sendFrame(byte[] frame);

	public native static boolean ConnextGstreamer_initializeSub(int domainID,
	        String peerList, boolean use_multicast);
	public native static boolean ConnextGstreamer_finalizeSub();
    public native static void ConnextGstreamer_setDisplayWindow(Surface window);
    public native static void ConnextGstreamer_unsetDisplayWindow();
    
	public native static void ConnextGstreamer_pause();
	public native static void ConnextGstreamer_play();
	
	public native static void ConnextGstreamer_initializeDisplay(int width, int height);
	public native static void ConnextGstreamer_finalizeDisplay();
	
	static {
		/* Loads librti_android_videodemo.so, native library implementing
		 * functions declared above.
		 */
		System.loadLibrary("gstreamer_android");
		System.loadLibrary("nddscore");
		System.loadLibrary("nddsc");
		System.loadLibrary("nddscpp");
		System.loadLibrary("rti_android_videodemo");
	}
}
