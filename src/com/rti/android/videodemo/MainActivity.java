/*****************************************************************************/
/*         (c) Copyright, Real-Time Innovations, All rights reserved.        */
/*                                                                           */
/*         Permission to modify and use for internal purposes granted.       */
/* This software is provided "as is", without warranty, express or implied.  */
/*                                                                           */
/*****************************************************************************/

package com.rti.android.videodemo;

import com.rti.android.videodemo.R;
import com.rti.android.videodemo.Settings;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.graphics.PorterDuff.Mode;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.SurfaceHolder;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.FrameLayout;
import android.widget.ImageButton;
import android.widget.RadioButton;
import android.widget.ToggleButton;

/**
 * An example full-screen activity that shows and hides the system UI (i.e.
 * status bar and navigation/system bar) with user interaction.
 * 
 * @see SystemUiHider
 */

public class MainActivity extends Activity implements SurfaceHolder.Callback
{
	private static final String TAG = "RTIVideoDemo";

    private VideoControlSurface mView = null;
    
	private ToggleButton runningState;
	private ImageButton  pauseButton;
	private ImageButton  playButton;
	private RadioButton  pubButton;
	private RadioButton  subButton;
	private FrameLayout  videoFrame;

	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		
		setContentView(R.layout.activity_main);
		
        // Add display of preview
        mView = new VideoCamera(this);
        videoFrame = (FrameLayout) findViewById(R.id.videoFrame);
        videoFrame.addView(mView);   

        
    	// All of the action buttons
		runningState =  (ToggleButton) findViewById(R.id.runningState);
    	playButton = (ImageButton) findViewById(R.id.play_button);
    	pauseButton = (ImageButton) findViewById(R.id.pause_button);
        pubButton = (RadioButton) findViewById(R.id.pub_button);
    	subButton = (RadioButton) findViewById(R.id.sub_button);
    	
    	// What to do when the "Start/Stop" button is clicked
    	runningState.setOnCheckedChangeListener(new OnCheckedChangeListener()
    	{
			@Override
			public void onCheckedChanged(CompoundButton buttonView,
					boolean isChecked) {

			    // isCheck == true if start, false if not
				if (isChecked) {
					if (!mView.initStreaming() ) {
						
				        // reset the button so that user can try to start again
				        buttonView.setChecked(false);
				        Log.e(TAG, "Could not initialize video streaming (DDS + Gstreamer).");
				        
					} else {
						mView.shouldStream(false);
					    setImageButtonEnabled(buttonView.getContext(),true, playButton, android.R.drawable.ic_media_play);
					    setImageButtonEnabled(buttonView.getContext(),false, pauseButton, android.R.drawable.ic_media_pause);

				        Log.i(TAG, "Initialized video streaming (DDS + Gstreamer).");
					}

				} else {
					if (!mView.stopStreaming()) {
						
				        // reset the button so that user can try to stop again
				        buttonView.setChecked(true);
				        Log.e(TAG, "Could not stop video streaming (DDS + Gstreamer).");
				        
					} else {
						
						// if successful, should set pause button to show paused state
						mView.shouldStream(false);
					    setImageButtonEnabled(buttonView.getContext(),false, playButton, android.R.drawable.ic_media_play);
					    setImageButtonEnabled(buttonView.getContext(),false, pauseButton, android.R.drawable.ic_media_pause);
				        Log.i(TAG, "Stopped video streaming (DDS + Gstreamer).");
					}
				}
			}
    	});
    	
    	// What to do when the "Play" button is clicked
        playButton.setOnClickListener(new OnClickListener() 
        {
        	@Override
            public void onClick(View v) {
			    mView.shouldStream(true);
			    setImageButtonEnabled(v.getContext(),false, playButton, android.R.drawable.ic_media_play);
			    setImageButtonEnabled(v.getContext(),true, pauseButton, android.R.drawable.ic_media_pause);
			}
    	});
        
    	// What to do when the "Pause" button is clicked
        pauseButton.setOnClickListener(new OnClickListener() 
        {
        	@Override
            public void onClick(View v) {
			    mView.shouldStream(false);
			    setImageButtonEnabled(v.getContext(),true, playButton, android.R.drawable.ic_media_play);
			    setImageButtonEnabled(v.getContext(),false, pauseButton, android.R.drawable.ic_media_pause);
			}
    	});
        
    	// What to do when the "Pub" button is clicked
        pubButton.setOnCheckedChangeListener(new OnCheckedChangeListener()
    	{
			@Override
			public void onCheckedChanged(CompoundButton buttonView,
					boolean isChecked) {
				
				if ( isChecked ) {
					// are we already publishing?
					if ( mView instanceof VideoCamera ) {
						return;
					}
					
					// switch to sub
			        
			        // first stop what we were doing
					runningState.setChecked(false);      
			        videoFrame.removeView(mView);
			        
			        // Create the camera view
			        mView = new VideoCamera(buttonView.getContext());
			        videoFrame.addView(mView);
				}
			}
    	});
        
    	// What to do when the "Sub" button is clicked
        subButton.setOnCheckedChangeListener(new OnCheckedChangeListener()
    	{	
			@Override
			public void onCheckedChanged(CompoundButton buttonView,
					boolean isChecked) {
				
				if ( isChecked ) {
					// are we already subscribing?
					if ( mView instanceof VideoDisplay ) {
						return;
					}
					
					// switch to sub
			        
			        // first stop what we were doing
					runningState.setChecked(false);      
			        videoFrame.removeView(mView);   
			        
			        // Create the video view
			        mView = new VideoDisplay(buttonView.getContext());
			        videoFrame.addView(mView);
				}
			}
    	});
        
	}
	
    public void surfaceCreated(SurfaceHolder holder) 
	{
		mView.surfaceCreated(holder);
	}
	
    public void surfaceChanged(SurfaceHolder holder, int format, int width,
			int height) 
    {
    	mView.surfaceChanged(holder, format, width, height);
    }
    
    public void surfaceDestroyed(SurfaceHolder holder) 
   	{
    	mView.surfaceDestroyed(holder);
   	}
    
	@Override
	protected void onStart()
	{
		// Start in streaming state
		runningState.setChecked(true);
		
		super.onStart();
	}
	
	@Override
	protected void onResume() 
	{
		// Start preview
        mView.startViewing();
        super.onResume();
	}
	
	@Override
	protected void onPause()
	{
		// Stop the streaming
		runningState.setChecked(false);
		
		// Stop preview
		mView.stopViewing();

		super.onPause();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu)
	{
		getMenuInflater().inflate(R.menu.menu, menu);
		
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item)
	{
		switch (item.getItemId()) {
			case R.id.menu_settings:
				startActivity(new Intent(this, Settings.class));
				return true;
			default:
				return false;
		}
	}
	
	/**
	 * Sets the specified image button to the given state, while modifying or
	 * "graying-out" the icon as well
	 * 
	 * @param enabled The state of the menu item
	 * @param item The menu item to modify
	 * @param iconResId The icon ID
	 */
	public static void setImageButtonEnabled(Context ctxt, boolean enabled, ImageButton item,
	        int iconResId) {
	    item.setEnabled(enabled);
	    Drawable originalIcon = ctxt.getResources().getDrawable(iconResId);
	    Drawable icon = enabled ? originalIcon : convertDrawableToGrayScale(originalIcon);
	    item.setImageDrawable(icon);
	}

	/**
	 * Mutates and applies a filter that converts the given drawable to a Gray
	 * image. This method may be used to simulate the color of disable icons in
	 * Honeycomb's ActionBar.
	 * 
	 * @return a mutated version of the given drawable with a color filter
	 *         applied.
	 */
	public static Drawable convertDrawableToGrayScale(Drawable drawable) {
	    if (drawable == null) {
	        return null;
	    }
	    Drawable res = drawable.mutate();
	    res.setColorFilter(Color.GRAY, Mode.SRC_IN);
	    return res;
	}
}
