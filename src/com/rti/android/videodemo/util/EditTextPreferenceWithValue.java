/*****************************************************************************/
/*         (c) Copyright, Real-Time Innovations, All rights reserved.        */
/*                                                                           */
/*         Permission to modify and use for internal purposes granted.       */
/* This software is provided "as is", without warranty, express or implied.  */
/*                                                                           */
/*****************************************************************************/
package com.rti.android.videodemo.util;

import com.rti.android.videodemo.R;

import android.content.Context;
import android.preference.EditTextPreference;
import android.util.AttributeSet;
import android.view.View;
import android.widget.TextView;

/* Useful class so that when a preference (setting/option) is changed by the user,
 * the new value is automatically reflected in the preferences menu.
 */
public class EditTextPreferenceWithValue extends EditTextPreference
{

	private TextView mValueText;

	public EditTextPreferenceWithValue(Context context, AttributeSet attrs)
	{
		super(context, attrs);
		setLayoutResource(R.layout.preference_with_value);
	}

	public EditTextPreferenceWithValue(Context context)
	{
		super(context);
		setLayoutResource(R.layout.preference_with_value);
	}

	@Override
	protected void onBindView(View view)
	{
		super.onBindView(view);
		mValueText = (TextView) view.findViewById(R.id.preference_value);
		if (mValueText != null) {
			mValueText.setText(getText());
		}
	}

	@Override
	public void setText(String text)
	{
		super.setText(text);
		if (mValueText != null) {
			mValueText.setText(getText());
		}
		notifyChanged();
	}

}