package com.rti.android.videodemo;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.util.AttributeSet;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public abstract class VideoControlSurface extends SurfaceView implements SurfaceHolder.Callback {

	protected boolean mShouldStream = false;
	
    public void shouldStream(boolean state) 
    {
    	mShouldStream = state;
		if ( state ) {
			startViewing();
		} else {
			stopViewing();
		}
    }
    
	public VideoControlSurface(Context context) {
		super(context);
	}

	public VideoControlSurface(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	public VideoControlSurface(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
	}

	public abstract void startViewing();
	
	public abstract void stopViewing();
	
    public abstract boolean initStreaming();

    public abstract boolean stopStreaming();
	
}
