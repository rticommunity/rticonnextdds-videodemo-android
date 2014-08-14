/*****************************************************************************/
/*         (c) Copyright, Real-Time Innovations, All rights reserved.        */
/*                                                                           */
/*         Permission to modify and use for internal purposes granted.       */
/* This software is provided "as is", without warranty, express or implied.  */
/*                                                                           */
/*****************************************************************************/

package com.rti.android.videodemo;

import java.io.IOException;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import com.rti.android.videodemo.VideoStreamConnext.VideoStreamConnextType;

import android.preference.PreferenceManager;
import android.util.Log;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.hardware.Camera.Size;
import android.view.SurfaceHolder;


public class VideoCamera extends VideoControlSurface implements Camera.PreviewCallback
{
    private Camera mCamera = null;
    private static final String TAG = "VideoCamera";
	private Size mSize;
	
    public VideoCamera(Context context) 
    {
        super(context);
        
        // Install a SurfaceHolder.Callback so we get notified when the
        // underlying surface is created and destroyed.
        SurfaceHolder holder = getHolder();
        holder.addCallback(this);
        
        mCamera = getCamera();
        
	    // Make sure the parameters were set
        if (mCamera != null) {
        	mSize = mCamera.getParameters().getPreviewSize();
        }
        
        Log.i(TAG, "VideoCamera created.");
    }
    
    @Override
    public void surfaceCreated(SurfaceHolder holder) 
	{
        // The Surface has been created, now tell the camera where to draw the preview.
        try {
        	Camera.Parameters params = null;
        	
        	if ( mCamera == null ) {
        		mCamera = getCamera();
        	}
            
            params = mCamera.getParameters();
            mSize = params.getPreviewSize();
            
            // Allocate buffer to hold preview data (avoids dynamic allocation)
            int length = mSize.width * mSize.height
            		* ImageFormat.getBitsPerPixel(params.getPreviewFormat()) / 8;
            
            // Setup the camera to callback on preview
            mCamera.addCallbackBuffer(new byte[length]);
            mCamera.setPreviewCallbackWithBuffer(this);
            mCamera.setPreviewDisplay(holder);
            
        } catch (IOException e) {
            Log.e(TAG, "Error starting video camera: " + e.getMessage());
        }
    }
	
    @Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width,
			int height) 
	{
        startViewing();
	}

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) 
	{
        stopViewing();
        stopStreaming();
        mCamera.setPreviewCallbackWithBuffer(null);
        mCamera.release();
        mCamera = null;
    }

    private static int counter = 0;
    @Override
    public void onPreviewFrame(byte[] data, Camera camera) 
    {
    	if (mShouldStream) {
    		// subsampling video stream for performance
    		if ( counter++ % 2 == 0 ) {
	    	    if (!VideoStreamConnext.write(data)) {
			        Log.e(TAG, "Could not send frame.");
				} else {
			        //Log.i(TAG, "Preview frame of length " + data.length);
				}
    		}
    	}
    	
    	// reuse the buffer, avoids allocation
        camera.addCallbackBuffer(data);
    }
	
    @Override
	public void startViewing()
	{
	   VideoStreamConnext.play();
       if ( mCamera != null ) {
    	   mCamera.startPreview();
       }
	}
	
    @Override
	public void stopViewing()
	{
       if ( mCamera != null ) {
    	   mCamera.stopPreview();
       }
       VideoStreamConnext.pause();
	}
	
    @Override
    public boolean initStreaming()
    {
		/* Get some settings from preference manager */
		SharedPreferences settings = PreferenceManager.getDefaultSharedPreferences(getContext());
        int domainID = Integer.parseInt(settings.getString("domainID", "0"));
        String peerList = settings.getString("peerList", "239.255.0.1");
		
		return VideoStreamConnext.initialize(VideoStreamConnextType.PUBLISHER, domainID, peerList, getContext(), 
				mSize.width, mSize.height, true);
    }
    
    @Override
    public boolean stopStreaming()
    {
		return VideoStreamConnext.stop();
    }
    
    private Camera getCamera()
    {
    	Camera camera = null;
    	
    	// Get a hold of the camera
		if (!getContext().getPackageManager().hasSystemFeature(PackageManager.FEATURE_CAMERA)){
	        // no camera on this device
	        Log.e(TAG, "No camera detected.");
	        return null;
	    }
	
	    try {
	    	camera = Camera.open();
	    }
	    catch (Exception e) {
	        Log.e(TAG, "Could not get camera : " + e.getMessage());
	        return null;
	    }

	    Camera.Parameters params = camera.getParameters();
	    
	    // limit size for performance
	    List<Size> previewSizes = params.getSupportedPreviewSizes();
	    Size max_size = camera.new Size(640, 480);
	    
	    if ( previewSizes.contains(max_size) ) { 
	    	Log.i(TAG,"Set camera preview size to " + max_size.width + " " + max_size.height);
	    	params.setPreviewSize(max_size.width, max_size.height);
	    }
	    else {
	    	// Sort list by decreasing width
	    	Collections.sort(previewSizes, new Comparator<Size>() 
    			{
		    		@Override
		    	    public int compare(Size o1, Size o2) {
		    			return (o1.width > o2.width ? -1 : (o1.width==o2.width ? 0 : 1));
		    		}
		    	}
	    	);
	    	// Assumes sizes sorted in decreasing order by width
	    	for (Size size : previewSizes) {
		    	if (max_size.width >= size.width) {
			    	Log.i(TAG,"Set camera preview size to " + size.width + " " + size.height);
		    		params.setPreviewSize(size.width,size.height);
		    		break;
		    	}
	    	}  
	    }
	    
	    List<int[]> ranges = params.getSupportedPreviewFpsRange();
	    int desiredFps = 15*1000;
	    
	    // Sort list by deceasing max fps
	    Collections.sort(ranges, new Comparator<int[]>() 
    		{
				@Override
			    public int compare(int o1[], int o2[]) {
					return (o1[1] > o2[1] ? -1 : (o1[1]==o2[1] ? 0 : 1));
				}
			}
	    );
	    
	    // Assumes ranges is sorted in decreasing max fps
	    for (int[] range : ranges) {
	    	if (desiredFps >= range[1]) {
	    		Log.i(TAG,"Set camera preview fps range to " + range[0] + " " + range[1]);
	    		params.setPreviewFpsRange(range[0], range[1]);
	    		break;
	    	}
	    }
	    
	    // Try to use the native NV21 preview format
	   	if ( params.getPreviewFormat() != ImageFormat.NV21 ) {
	   		
	   		List<Integer> formats = params.getSupportedPreviewFormats();
	   	
	   		if (formats.contains(ImageFormat.NV21)) {
	   			params.setPreviewFormat(ImageFormat.NV21);
	   		} else {
	   			Log.e(TAG, "Preview pixel format NV21 not supported by device.  Using format "
	   					+ params.getPreviewFormat());
	   		}
	   	}
	   	
	   	// Change camera parameters
	   	camera.setParameters(params);
	    
		return camera;
    }
    
}
