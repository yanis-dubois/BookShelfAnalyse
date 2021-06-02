package com.renard.ocr.main_menu;

import com.renard.ocr.MonitoredActivity;
import com.renard.ocr.R;

import android.os.Bundle;
import android.view.MenuItem;
import android.view.View;

import androidx.core.app.NavUtils;

public class AccessibilityActivity extends MonitoredActivity implements View.OnClickListener {

    private boolean slideOutLeft = false;

    @Override
    public String getScreenName() { return "accessibility"; }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_accessibility);
        initToolbar();
        setToolbarMessage(R.string.accessibility);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        findViewById(R.id.button_accessibility).setOnClickListener(this);
    }

    @Override
    protected int getHintDialogId() { return -1; }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                NavUtils.navigateUpFromSameTask(this);
                return true;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (slideOutLeft) {
            overridePendingTransition(R.anim.slide_in_right, R.anim.slide_out_left);
        } else {
            overridePendingTransition(R.anim.slide_in_left, R.anim.slide_out_right);
        }
    }

    @Override
    public void onClick(View v) {

    }
}