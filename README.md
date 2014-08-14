# RTI Video Demo For Android, rtivideodemo-android

An Android application that uses RTI Connext DDS and the open source
gstreamer multimedia framework to send a live video stream from one
Android device to another.

You can get the project for this demo with a precompiled .apk from
github, https://github.com/rticommunity.

This demo is also compatible with the Distributed Video Demo that
shows video streaming on a PC using RTI Connext.  That demo is
available from
http://www.rti.com/resources/usecases/streaming-video.html.

NOTE: This demo uses GStreamer 1.0 to encode (compress) the raw video
stream received from the Android device's camera using VP8.  VP8 is
the compression method that is used by WebM, which is use by
HTML5. http://www.webmproject.org/.  

NOTE: You will see a noticeable delay in the capture of the video on
one device to the display on another device.  The source of this delay
is entirely within the VP8 encoding/decoding done entirely by
GStreamer 1.0 VP8 software plugins.  There is no delay due to the use
of DDS itself.

If the demo was modified to take advantage of the hardware video
processing capability offered by many modern video devices (phone and
PC web cameras), there would be noticeable decrease in the delay.
However, this demo was created to run on any generic Android device
and does not require specific hardware video-processing support.


## Getting Started
-------------------------------------------------------------------

1. Installing `rtivideodemo-android.apk` on a real Android Device

   It will show as the "Connext Video Demo" icon.

    A. Attach your Device via USB (to a PC where you have the .apk file)
    
    B. Enable your Device to install apps from unknown sources. 
      On Device, use Settings -> Security -> Unknown Sources, 
      check the checkbox
        
    C. Optional, enable your Device to allow debugging over USB
      On Device, use Settings -> Developer options -> USB debugging,
      check the checkbox.
      NOTE: You may need to enable the Developer options menu option
      on some devices. e.g., on Nexus 7, use Settings -> About tablet
      -> Build number, select Build number 10 times (or so)
        
    D. Installing the .apk
    
        1. Using the rtivideodemo-android.apk directly
        
          The .apk is located in 'bin/rtivideodemo-android.apk' in the
          rtivideodemo-android project.

          You install the rtivideodemo-android.apk by either

          * Copying 'rtivideodemo-android.apk' to your Device's
            filesystem mounted via USB. 
            Use a file manager app (which you'll need to install from
            Google Play, many free ones available) to select and
            install the .apk on your Device.
            Select the installed rtivideodemo-android icon to run.

          * or from a command line on your PC, running: 
        
                  adb install rtivideodemo-android.apk
        
            See http://developer.android.com/tools/help/adb.html
            'adb.exe' must be executable from your command line.


2. Using Eclipse
          
   From the toolbar use Run -> Run . This will automatically build the
   project (so project must successfully build, see #Building below)
     
   The dialog, Android Device Chooser, may popup and show your device
   if you have more than 1 Android device connected via USB.

   If an Android device emulator pops up or if your device is not
   shown as "online", you probably need to go to the Device itself
   which has popped up a dialog asking you for permission to allow
   your computer to connect via USB for debugging. (note, while the
   demo will start in an emulator, it will not be able to connect to
   another device since the emulator's network setup is not the same
   as a real device)

   Select your device in the Android Device Chooser if needed.

   The `rtivideodemo-android.apk` will automatically be installed and
   started.

   You can use the CDT Build Console and the Android Console to
   monitor the build/installation process.  You can open a LogCat
   console to see the debug messages coming from the
   rtivideodemo-android application.


## Running
-------------------------------------------------------------------

1. "Connext Video Demo" is configured to work using Android's WiFi
   network interface, so your device should be connected to a WiFi
   network.
   
   NOTE: The demo uses RTI Connext DDS to send a video stream to
   another Android device or to a PC via the WiFi.  This means that
   your device must be able to connect to another device (or a PC) via
   the WiFi.  If you are on a WiFi that requires a login via a web
   page after initial connection (for example at a hotel or conference
   hall, or using a roaming service such as Boingo), those types of
   connections actually don't allow one device connected to the WiFi
   from directly connecting to another device on the same WiFi.  So
   this demo will not work in that setup.

   It will work on a local WiFi router (even if the WiFi connection
   requires a WEP or WPA authentication key).

   NOTE: If your Android devices support WiFfi-Direct (for example
   a Nexus 7), http://en.wikipedia.org/wiki/Wi-Fi_Direct, you can use
   this to connect your devices directly to each other via WiFi and
   have the demo stream from one device to another.
    

2. You will need at least 2 devices running "Connext Video Demo" or a
   compatible video demo on a PC, for example from
   http://www.rti.com/resources/usecases/streaming-video.html.


3. Start the "Connext Video Demo" on both devices.  By default, the
   demo will start in Publish mode, which means that it will attach to
   the video camera of the device and publish a video stream via RTI
   Connext DDS.  So, you will have to change one of the devices to
   Subscribe mode so that it will receive and display the video stream
   from the other device.

   In the app, select "Sub".  Wait until the app switches to "Sub" and
   then hit the "Start" button which will subscribe to the data
   stream.  To view the stream, you will have to hit the "Play" arrow
   button.

   You may need to start the publishing app by hitting the "Start"
   button as well as the "Play" arrow to begin streaming.


4. See Troubleshooting section below if you can't get a video stream
   from one app to the another.
    
    
## Building
-------------------------------------------------------------------

These instructions are for a Windows development host.

1. You need to have Eclipse and the Android ADT plugin installed. The
   Android ADT plugin depends on the Android SDK. You can:

    A. Install the Eclipse ADT Bundle, which has Eclipse with ADT in a
       single installer.
     
       Use this if you don't have Eclipse on your computer yet. Look
       for the Download the [SDK ADT Bundle for Windows
       link](http://developer.android.com/sdk/index.html) and follow
       [these instructions]
       (http://developer.android.com/sdk/installing/bundle.html).
    
    B. Install the ADT plugin and Android SDK separately.
    
       Use this if you already have an Eclipse installation that you
       want to use. Look for the ["Use an existing IDE"
       link](http://developer.android.com/sdk/index.html) and follow
       [these instructions]
       (http://developer.android.com/sdk/installing/index.html).

    C. After installation, if in Eclipse, you don't see the Android SDK, 
       manager and Android Virtual Device Manager menu options under
       the Window toolbar menu,
       then open Window -> Customize Perspective -> Command Groups 
       Availability tab, check Android SDK and AVD Manager.
       

2. Use the Android SDK Manager to install the Android platforms for
   which you want to build (e.g., Android 4.1.2, Android 4.2.2,
   Android 4.3, etc.).  Make use that you run the Android SDK Manager
   with Adminstrator Priviledges if on Windows, or else you won't be able
   to install any of the SDKs.


3. Install the Android NDK (native cross-compiler for Android/ARMv7).
   Needed to compile C/C++ code invoked through JNI on Android.

   NDK 9 or higher is required.

   Go to http://developer.android.com/tools/sdk/ndk/index.html and
   follow [these instructions]
   (http://tools.android.com/recent/usingthendkplugin).
   Note that you don't need the legacy NDK toolchains.


4. Install RTI Connext DDS (and Android target libraries)

   You'll need to get this from your local RTI representative. Contact
   info@rti.com to find out who this is.


5. Download and install GStreamer libraries for Android

   rtivideodemo-android works with gstreamer 1.3.90 and higher.


6. Open rtivideodemo-android project in Eclipse

    A. Start Eclipse

    B. Use File -> Import -> Existing Projects into Workspace

    C. Select the directory where the project rtivideodemo-android is
       located.


7. Customize the project to reflect your environment.

   A. Right click on rtivideodemo-android folder in Project Explorer,
   
      Open Properties -> C/C++ Build -> Environment
   
   B. Change the value of NDDSHOME and GSTREAMER_SDK_ROOT_ANDROID to
      reflect where you installed RTI Connext DDS and GGtreamer

   C. If you haven't done so already, you also need to make sure that 
      Eclipse knows where your Android NDK is installed.

      Open Window -> Preferences -> Android -> NDK, and edit NDK Location
   

8. Build Project

   Use Project -> Build Project. The Output in the (CDT Build) Console
   should look like:

   13:42:24 **** Incremental Build of configuration Default for
   project rtivideodemo-android ****

   "C:\\Apps\\android-ndk-r9\\ndk-build.cmd" all 
   GStreamer      : [GEN] => gst-build-armeabi-v7a/gstreamer_android.c
   GStreamer      : [COMPILE] => gst-build-armeabi-v7a/gstreamer_android.c
   GStreamer      : [LINK] => gst-build-armeabi-v7a/libgstreamer_android.so
   Prebuilt       : libgstreamer_android.so <= gst-build-armeabi-v7a/
   Install        : libgstreamer_android.so => libs/armeabi-v7a/libgstreamer_android.so
   Install        : libnddsc.so => libs/armeabi-v7a/libnddsc.so
   Install        : libnddscore.so => libs/armeabi-v7a/libnddscore.so
   Install        : libnddscpp.so => libs/armeabi-v7a/libnddscpp.so
   "Compile++ thumb : rti_android_videodemo <= ConnextGstreamer.cxx
   SharedLibrary  : librti_android_videodemo.so
   Install        : librti_android_videodemo.so => libs/armeabi-v7a/librti_android_videodemo.so
   Install        : libgnustl_shared.so => libs/armeabi-v7a/libgnustl_shared.so
        
13:42:43 Build Finished (took 18s.243ms)


9. Install APK and run on Android device

   Use Run -> Run from the Eclipse toolbar, or right click on the
   project and use Run As -> Android Application.  This will kick off
   a build and then automatically install the .apk on to your Android
   device and start the app.

   If you have more than one Android device plugged in, you may need to
   select the device that you want to run on from the Android Device
   Chooser that will pop up.

   If your device does not show up, or Eclipse starts an Android
   Emulator, it is likely that you need to configure your device to
   allow for USB debugging and likely to select "Allow" on your device
   when your device detects your computer trying to establish a USB
   debug session.

   NOTE: While the demo will start in an Android emulator, it will not
   be able to connect to another device since the emulator's network
   setup is not the same as a real device.

   You can use the CDT Build Console and the Android Console to
   monitor the build/installation process.  You can open a LogCat
   console to see the debug messages coming from the
   rtivideodemo-android application.

   Example Android Console output:

  [2014-08-11 14:09:36 - rtivideodemo-android] ------------------------------
  [2014-08-11 14:09:36 - rtivideodemo-android] Android Launch!
  [2014-08-11 14:09:36 - rtivideodemo-android] adb is running normally.
  [2014-08-11 14:09:36 - rtivideodemo-android] Performing com.rti.android.videodemo.MainActivity activity launch
  [2014-08-11 14:09:36 - rtivideodemo-android] Automatic Target Mode: using device 'SH18JT501845'
  [2014-08-11 14:09:36 - rtivideodemo-android] Uploading rtivideodemo-android.apk onto device 'SH18JT501845'
  [2014-08-11 14:09:39 - rtivideodemo-android] Installing rtivideodemo-android.apk...
  [2014-08-11 14:10:12 - rtivideodemo-android] Success!
  [2014-08-11 14:10:12 - rtivideodemo-android] Starting activity com.rti.android.videodemo.MainActivity on device SH18JT501845
  [2014-08-11 14:10:16 - rtivideodemo-android] ActivityManager: Starting: Intent { act=android.intent.action.MAIN cat=[android.intent.category.LAUNCHER] cmp=com.rti.android.videodemo/.MainActivity }


## Troubleshooting
-------------------------------------------------------------------  

1. rtivideodemo-android doesn't seem to connect (you don't see a live
   video feed sent from one device to another)

   If your Android devices on are different subnets or multicast is not
   supported on the WiFi network, then you should use
   `NDDS_DISCOVERY_PEERS` environment variable to set the IP addresses
   of the devices as the initial peers for discovery.

   An Android device IP address on WiFi can be found using Settings
   -> WiFi -> `< whatever WiFi network is connected >`.

    You can set the IP address in the Peer List for the demo on the
    Android device using the "..." (3 dots menu) -> Settings -> DDS
    Peer List preferences menu option.
    
   NOTE: If the Android devices are connected to a WiFi that requires
   a login (I don't mean a password for WEP or WPA/PSK WiFi security),
   then it is likely that your two devices will not be able to connect
   to each other.  This is true for most conference center, hotel or
   roaming WiFi services like Boingo.  Those WiFi services simply
   don't allow you to directly connect two devices on their network.


2. Build problems?

    A. Make sure that Eclipse knows where your NDK is located

       Set Eclipse -> Window -> Preferences -> Android -> NDK to the
       location of your Android NDK installation
    
    B. Your build never completes...and you have to kill Eclipse (or
       at least kill the build processes that were spawned by Eclipse).

       Not sure why, but the Scanner...that Eclipse runs for C/C++
       which basically creates the information needed for the Eclipse
       editor to flag syntax and other coding errors that usually is
       only found during compilation, but now can be dynamically
       detected if the code scanner invoked by Eclipse does its
       job...gets stuck and never finishes.

       Solution, disable the scanner.

       Open the properties for the rtivideodemo-android project ->
       Builders -> unselect Scanner Configuration Builder.

    C. After disabling your Scanner Configuration Builder, your
       project now complains that there are errors in the code after a
       build and now refuses to create, install and run an .apk.  Since 
       disabling the Scanner now prevents the Eclipse editor from
       successfuly verifying the syntax of your C/C++ code, it now
       flags unknown variable types and such as errors (which really
       aren't since the compiler is the one will work fine).

       To prevent the editor from flagging syntax errors that really
       aren't errors,

       Open the properties for the rtivideodemo-android project ->
       C/C++ General -> Code Analysis 
       -> uncheck Syntax and Semantic Errors.

            
## File Descriptions
-------------------------------------------------------------------

### Java ###
 
* 'src/com/rti/android/videodemo/MainActivity.java' - The main
  Android activity for rtivideodemo-android. Handles all of the button
  actions that switches between video camera and video display modes.
 
* 'src/com/rti/android/videodemo/VideoCamera.java' - Class that runs 
  the demo when in Camera (Pub) mode.  Connects the device camera to
  the underlying Connext DDS/GStreamer processing pipeline.
 
* 'src/com/rti/android/videodemo/VideoDisplay.java' - Class that runs
  the demo when in Display (Sub) mode.  Connects the underlying
  Connext DDS/GStreamer processing pipeline to a display surface to
  display the incoming video stream.

* 'src/com/rti/android/videodemo/VideoControlSurface.java' - Abstract
  class that serves as the base class for VideoCamera and
  VideoDisplay.  Allows buttons in the MainActivity to function
  correctly independently of the mode that the demo is in.

* 'src/com/rti/android/videodemo/VideoStreamConnext.java' - Class that 
  wraps the underlying C functions provided by the
  ConnextGstreamer.cxx file into equivalent Java methods via JNI.
 
* 'src/com/rti/android/videodemo/Settings.java' - Android Activity used
  to create "Settings" option menu.

* 'src/com/rti/android/videodemo/util/EditTextPreferenceWithValue.java' - 
  Useful class so that when a preference (setting/option) is changed
  by the user, the new value is automatically reflected in the
  preferences menu.
 
 
### JNI - C/C++ ###

* 'jni/ConnextGstreamer.cxx' - Wrapper using RTI Connext DDS and GStreamer 
  invoked via JNI interface.  The wrapper creates DDS entities such as
  DDSParticipants, DDSDataWriters, DDSDataReaders as well as creates
  the GStreamer video processing pipelines.  You would modify this
  file if you want to try different video compression algorithms or
  try to use a device's onboard video encoding/decoding hardware.

* 'jni/VideoData*.*' - Support files for data type use by DDS.  Generated 
  with "rtiddsgen VideoDemo.idl" using rtiddsgen distributed with RTI Connext DDS.

* 'jni/Android.mk', 'jni/Application.mk' - Defines the build process and 
  external libraries needed by the native code (RTI Connext DDS and
  GStreamer) supporting the demo.

* 'VideoData.idl' - Defines the data structure used by DDS to stream video frames
   over a network.
 

### Android Application Files ###

* 'AndroidManifest.xml' - Main definition file for rtivideodemo-android app 
  including which Activity to start on startup.

* 'res/layout/activity_main.xml' - Main layout of the graphics elements of
  the application.  Open this file to modify the graphical elements of the 
  application.

* 'res/layout/preferences_with_value.xml' - Utility used to define a layout 
  where the current value of a preference are displayed on the same line as 
  the preference.

* 'res/menu/menu.xml' - Defines the layout of the Android options menu.

* 'res/xml/settings.xml' - Defines the preferences that can be set by the 
  Settings option submen.


