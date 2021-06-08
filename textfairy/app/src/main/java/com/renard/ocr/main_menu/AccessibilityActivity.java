package com.renard.ocr.main_menu;

import com.renard.ocr.MonitoredActivity;
import com.renard.ocr.R;
import com.renard.ocr.util.PreferencesUtils;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.provider.OpenableColumns;
import android.util.Log;
import android.view.MenuItem;
import android.view.View;

import androidx.annotation.Nullable;
import androidx.core.app.NavUtils;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class AccessibilityActivity extends MonitoredActivity implements View.OnClickListener {

    private boolean slideOutLeft = false;
    private static final int PICK_FONT_REQUEST_CODE = 1;

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

        applyFont();
    }

    private void saveData(String path) {
        SharedPreferences sharedPreferences = getSharedPreferences(PreferencesUtils.LAST_FONT, MODE_PRIVATE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putString(PreferencesUtils.FONT, path);
        editor.apply();

        applyFont();
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
        switch (v.getId()) {
            case R.id.button_accessibility:
                Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
                intent.setFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION);
                intent.setType("*/*");
                // only (.ttf, .ttc, .otf, .xml) font can be used
                if (intent.resolveActivity(getPackageManager()) != null) {
                    startActivityForResult(intent, PICK_FONT_REQUEST_CODE);
                } else {
                    Log.d("Accessibility Activity","Unable to resolve Intent.ACTION_OPEN_DOCUMENT {}");
                }
                break;
        }
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        switch (requestCode) {
            case PICK_FONT_REQUEST_CODE: {
                if (resultCode == Activity.RESULT_OK) {
                    if (data != null && data.getData() != null) {
                        new CopyFileToAppDirTask().execute(data.getData());
                    } else {
                        Log.d("Accessibility Activity","File uri not found {}");
                    }
                } else {
                    Log.d("Accessibility Activity","User cancelled file browsing {}");
                }
                break;
            }
        }
    }

    public static final String FILE_BROWSER_CACHE_DIR = "CertCache";

    @SuppressLint("StaticFieldLeak")
    private class CopyFileToAppDirTask extends AsyncTask<Uri, Void, String> {
        private ProgressDialog mProgressDialog;

        private CopyFileToAppDirTask() {
            mProgressDialog = new ProgressDialog(AccessibilityActivity.this);
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            mProgressDialog.setMessage("Please Wait..");
            mProgressDialog.show();
        }

        protected String doInBackground(Uri... uris) {
            try {
                return writeFileContent(uris[0]);
            } catch (IOException e) {
                Log.e("", "Failed to copy file {}" + e.getMessage());
                return null;
            }
        }

        protected void onPostExecute(String cachedFilePath) {
            mProgressDialog.dismiss();
            if (cachedFilePath != null) {
                Log.d("", "Cached file path {}" + cachedFilePath);
            } else {
                Log.d("", "Writing failed {}");
            }
        }
    }

    private String writeFileContent(final Uri uri) throws IOException {
        InputStream selectedFileInputStream =
                getContentResolver().openInputStream(uri);
        if (selectedFileInputStream != null) {
            final File certCacheDir = new File(getExternalFilesDir(null), FILE_BROWSER_CACHE_DIR);
            boolean isCertCacheDirExists = certCacheDir.exists();
            if (!isCertCacheDirExists) {
                isCertCacheDirExists = certCacheDir.mkdirs();
            }
            if (isCertCacheDirExists) {
                String filePath = certCacheDir.getAbsolutePath() + "/" + getFileDisplayName(uri);
                OutputStream selectedFileOutPutStream = new FileOutputStream(filePath);
                byte[] buffer = new byte[1024];
                int length;
                while ((length = selectedFileInputStream.read(buffer)) > 0) {
                    selectedFileOutPutStream.write(buffer, 0, length);
                }
                selectedFileOutPutStream.flush();
                selectedFileOutPutStream.close();

                // -- save path -- //
                saveData(filePath);
                return filePath;
            }
            selectedFileInputStream.close();
        }
        return null;
    }

    // Returns file display name.
    @Nullable
    private String getFileDisplayName(final Uri uri) {
        String displayName = null;
        try (Cursor cursor = getContentResolver()
                .query(uri, null, null, null, null, null)) {
            if (cursor != null && cursor.moveToFirst()) {
                displayName = cursor.getString(
                        cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME));
                Log.i("", "Display Name {}" + displayName);
            }
        }
        return displayName;
    }

}
