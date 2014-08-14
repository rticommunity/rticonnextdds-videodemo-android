LOCAL_PATH := $(call my-dir)

NDDSARCH := armv7aAndroid2.3gcc4.8

ifndef NDDSHOME
$(error NDDSHOME is not defined!)
endif
ifndef GSTREAMER_SDK_ROOT_ANDROID
$(error GSTREAMER_SDK_ROOT_ANDROID is not defined!)
endif


include $(CLEAR_VARS)
LOCAL_MODULE := nddscore
LOCAL_SRC_FILES := $(NDDSHOME)/lib/$(NDDSARCH)/libnddscore.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := nddsc
LOCAL_SRC_FILES := $(NDDSHOME)/lib/$(NDDSARCH)/libnddsc.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := nddscpp
LOCAL_SRC_FILES := $(NDDSHOME)/lib/$(NDDSARCH)/libnddscpp.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE    := rti_android_videodemo
LOCAL_C_INCLUDES += $(NDDSHOME)\include\ndds $(NDDSHOME)\include
LOCAL_CFLAGS    := -DRTI_UNIX -DRTI_ANDROID -mlong-calls
LOCAL_SRC_FILES := ConnextGstreamer.cxx
LOCAL_SRC_FILES += VideoData.cxx VideoDataPlugin.cxx VideoDataSupport.cxx 
LOCAL_SHARED_LIBRARIES := gstreamer_android nddscpp nddsc nddscore 
LOCAL_LDLIBS :=  -llog -landroid 

include $(BUILD_SHARED_LIBRARY)

GSTREAMER_ROOT             := $(GSTREAMER_SDK_ROOT_ANDROID)
GSTREAMER_NDK_BUILD_PATH   := $(GSTREAMER_ROOT)/share/gst-android/ndk-build/

include $(GSTREAMER_NDK_BUILD_PATH)/plugins.mk

GSTREAMER_PLUGINS          := coreelements videoconvert videoscale app autodetect vpx opensles opengl 
GSTREAMER_EXTRA_DEPS       := gstreamer-video-1.0

include $(GSTREAMER_NDK_BUILD_PATH)/gstreamer-1.0.mk 

