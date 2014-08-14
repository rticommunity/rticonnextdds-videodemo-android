/*****************************************************************************/
/*         (c) Copyright, Real-Time Innovations, All rights reserved.        */
/*                                                                           */
/*         Permission to modify and use for internal purposes granted.       */
/* This software is provided "as is", without warranty, express or implied.  */
/*                                                                           */
/*****************************************************************************/

package com.rti.android.videodemo;

import android.preference.PreferenceManager;
import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;
import android.view.SurfaceHolder;

import com.rti.android.videodemo.VideoStreamConnext.VideoStreamConnextType;

public class VideoDisplay extends VideoControlSurface
{
    private static final String TAG = "VideoDisplay";
    private boolean initialized = false;
	
    public VideoDisplay(Context context) 
    {
        super(context);
        
        // Install a SurfaceHolder.Callback so we get notified when the
        // underlying surface is created and destroyed.
        SurfaceHolder holder = getHolder();
        holder.addCallback(this);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) 
	{
    }
	
    @Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width,
			int height) 
	{

       if ( !initialized ) {
           VideoStreamConnext.initializeDisplay(width, height);
           initialized = true;
           Log.i(TAG, "VideoDisplay created.");
       }
       
	   VideoStreamConnext.setDisplaySurface(holder.getSurface());
       startViewing();
	}

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) 
	{
        stopViewing();
        stopStreaming();
        VideoStreamConnext.unsetDisplaySurface();
        VideoStreamConnext.finalizeDisplay();
        initialized = false;
    }
	
    @Override
	public void startViewing()
	{
    	VideoStreamConnext.play();
	}
	
    @Override
	public void stopViewing()
	{
    	VideoStreamConnext.pause();
	}
	
    @Override
    public boolean initStreaming()
    {
		/* Get some settings from preference manager */
		SharedPreferences settings = PreferenceManager.getDefaultSharedPreferences(getContext());
        int domainID = Integer.parseInt(settings.getString("domainID", "0"));
        String peerList = settings.getString("peerList", "239.255.0.1");
        boolean use_multicast = settings.getBoolean("use_multicast_check", true);
        
		return VideoStreamConnext.initialize(VideoStreamConnextType.SUBCRIBER, domainID, peerList, getContext(), 0, 0, use_multicast);
    }
    
    @Override
    public boolean stopStreaming()
    {
		return VideoStreamConnext.stop();
    }
   
	
}
