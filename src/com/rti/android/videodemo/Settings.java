/*****************************************************************************/
/*         (c) Copyright, Real-Time Innovations, All rights reserved.        */
/*                                                                           */
/*         Permission to modify and use for internal purposes granted.       */
/* This software is provided "as is", without warranty, express or implied.  */
/*                                                                           */
/*****************************************************************************/

package com.rti.android.videodemo;

import com.rti.android.videodemo.R;

import android.app.Activity;
import android.os.Bundle;
import android.preference.PreferenceFragment;

/* Android Activity used to create "Settings" option */
public class Settings extends Activity
{
	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		
        // Display the fragment as the main content.
        getFragmentManager().beginTransaction()
                .replace(android.R.id.content, new SettingsMenu())
                .commit();

	}
	
	/**
	 * This fragment creates and populates the "Settings" option
	 */
	public static class SettingsMenu extends PreferenceFragment
	{
		@Override
		public void onCreate(Bundle savedInstanceState)
		{
			super.onCreate(savedInstanceState);

			// Load the preferences from an XML resource
			addPreferencesFromResource(R.xml.settings);
		}
	}

}