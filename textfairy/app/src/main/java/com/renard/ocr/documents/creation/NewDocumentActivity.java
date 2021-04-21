/*
 * Copyright (C) 2012,2013 Renard Wellnitz.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

package com.renard.ocr.documents.creation;

import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.ActivityNotFoundException;
import android.content.ClipData;
import android.content.ContentProviderClient;
import android.content.ContentValues;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.RemoteException;
import android.provider.MediaStore;
import android.text.Html;
import android.text.Spanned;
import android.util.Log;
import android.util.Pair;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import androidx.core.content.FileProvider;

import com.renard.ocr.MonitoredActivity;
import com.renard.ocr.R;
import com.renard.ocr.documents.creation.ocr.OCRActivity;
import com.renard.ocr.documents.creation.ocr.OcrPdfActivity;
import com.renard.ocr.documents.viewing.DocumentContentProvider;
import com.renard.ocr.documents.viewing.DocumentContentProvider.Columns;
import com.renard.ocr.documents.viewing.grid.DocumentGridActivity;
import com.renard.ocr.documents.viewing.single.DocumentActivity;
import com.renard.ocr.main_menu.language.OcrLanguageDataStore;
import com.renard.ocr.main_menu.language.OcrLanguageListActivity;
import com.renard.ocr.pdf.Hocr2Pdf;
import com.renard.ocr.pdf.Hocr2Pdf.PDFProgressListener;
import com.renard.ocr.util.AppStorage;
import com.renard.ocr.util.MemoryInfo;
import com.renard.ocr.util.Util;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Set;

import static com.renard.ocr.documents.viewing.single.DocumentActivity.EXTRA_ACCURACY;
import static com.renard.ocr.documents.viewing.single.DocumentActivity.EXTRA_LANGUAGE;

/**
 * activities which extend this activity can create a new document. this class
 * also contains the code for functionality which is shared by
 * {@link DocumentGridActivity} and {@link DocumentActivity}
 *
 * @author renard
 */
public abstract class NewDocumentActivity extends MonitoredActivity {

    private final static String LOG_TAG = NewDocumentActivity.class.getSimpleName();


    private static final int PDF_PROGRESS_DIALOG_ID = 0;
    private static final int DELETE_PROGRESS_DIALOG_ID = 1;
    protected static final int HINT_DIALOG_ID = 2;
    private static final int EDIT_TITLE_DIALOG_ID = 3;

    private static final String DIALOG_ARG_MAX = "max";
    private static final String DIALOG_ARG_MESSAGE = "message";
    private static final String DIALOG_ARG_PROGRESS = "progress";
    private static final String DIALOG_ARG_SECONDARY_PROGRESS = "secondary_progress";
    private static final String DIALOG_ARG_TITLE = "title";
    private static final String DIALOG_ARG_DOCUMENT_URI = "document_uri";

    private final static int REQUEST_CODE_MAKE_PHOTO = 0;
    private final static int REQUEST_CODE_PICK_PHOTO = 1;

    protected final static int REQUEST_CODE_OCR = 3;

    private static final String DATE_CAMERA_INTENT_STARTED_STATE = "com.renard.ocr.android.photo.TakePhotoActivity.dateCameraIntentStarted";
    public static final String EXTRA_IMAGE_SOURCE = "image_source";
    private static Date dateCameraIntentStarted = null;
    private static final String CAMERA_PIC_URI_STATE = "com.renard.ocr.android.photo.TakePhotoActivity.CAMERA_PIC_URI_STATE";
    private static final String CAMERA_PIC_LOCAL_URI_STATE = "com.renard.ocr.android.photo.TakePhotoActivity.CAMERA_PIC_LOCAL_URI_STATE";

    private static Uri cameraPicUri = null;
    private static Uri localCameraPicUri = null;

    private static class CameraResult {
        public CameraResult(int requestCode, int resultCode, Intent data) {
            mRequestCode = requestCode;
            mResultCode = resultCode;
            mData = data;
        }

        private final int mRequestCode;
        private final int mResultCode;
        private final Intent mData;
    }

    protected abstract int getParentId();

    private ProgressDialog pdfProgressDialog;
    private ProgressDialog deleteProgressDialog;
    private CameraResult mCameraResult;

    private void checkRam(MemoryWarningDialog.DoAfter doAfter) {
        long availableMegs = MemoryInfo.getFreeMemory(this);
        Log.i(LOG_TAG, "available ram = " + availableMegs);
        if (availableMegs < MemoryInfo.MINIMUM_RECOMMENDED_RAM) {
            MemoryWarningDialog.newInstance(availableMegs, doAfter).show(getSupportFragmentManager(), MemoryWarningDialog.TAG);
        } else if (doAfter == MemoryWarningDialog.DoAfter.START_CAMERA) {
            startCamera();
        } else if (doAfter == MemoryWarningDialog.DoAfter.START_GALLERY) {
            startGallery();
        }
    }

    protected void startGallery() {
        ensureLanguageInstalled(() -> {
            mAnalytics.startGallery();
            cameraPicUri = null;
            Intent i;
            if (Build.VERSION.SDK_INT >= 19) {
                i = new Intent(Intent.ACTION_OPEN_DOCUMENT, null);
                //TODO enable
                //i.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, true);
                i.addCategory(Intent.CATEGORY_OPENABLE);
                i.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION);
                i.setType("*/*");
                i.putExtra(Intent.EXTRA_MIME_TYPES, new String[]{"image/png", "image/jpg", "image/jpeg", "application/pdf"});
            } else {
                i = new Intent(Intent.ACTION_GET_CONTENT, null);
                i.setType("image/png,image/jpg, image/jpeg");
            }

            Intent chooser = Intent.createChooser(i, getString(R.string.image_source));
            try {
                startActivityForResult(chooser, REQUEST_CODE_PICK_PHOTO);
            } catch (ActivityNotFoundException e) {
                Toast.makeText(this, R.string.no_gallery_found, Toast.LENGTH_LONG).show();
            }
        });

    }

    protected void startCamera() {
        ensureLanguageInstalled(() -> {
            mAnalytics.startCamera();
            try {
                cameraPicUri = null;
                dateCameraIntentStarted = new Date();
                Intent intent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
                // Create an image file name
                String timeStamp = new SimpleDateFormat("yyyyMMdd_HHmmss", Locale.US).format(new Date());
                String imageFileName = "JPEG_" + timeStamp + ".jpg";

                File dir = new File(getCacheDir(), getString(R.string.config_share_file_dir));
                dir.mkdirs();
                File image = new File(dir, imageFileName);
                cameraPicUri = toUri(image);
                localCameraPicUri = Uri.fromFile(image);
                intent.putExtra(MediaStore.EXTRA_OUTPUT, cameraPicUri);
                intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
                startActivityForResult(intent, REQUEST_CODE_MAKE_PHOTO);
            } catch (ActivityNotFoundException e) {
                showFileError(this, PixLoadStatus.CAMERA_APP_NOT_FOUND);
            }
        });

    }

    protected void ensureMediaMounted(Runnable block) {
        final String state = Environment.getExternalStorageState();
        if (state.equals(Environment.MEDIA_MOUNTED)) {
            block.run();
        } else {
            getNoSdCardDialog(this).show();
        }
    }

    private void ensureLanguageInstalled(Runnable block) {
        ensureMediaMounted(() -> {
            if (OcrLanguageDataStore.getInstalledOCRLanguages(getApplicationContext()).isEmpty()) {
                mAnalytics.noLanguageInstalled();
                getInstallLanguageDialog(this).show();
            } else {
                block.run();
            }
        });
    }

    private androidx.appcompat.app.AlertDialog getInstallLanguageDialog(Context context) {
        return new androidx.appcompat.app.AlertDialog.Builder(context)
                .setTitle(R.string.install_language)
                .setPositiveButton(R.string.install, (dialogInterface, i) -> startActivity(new Intent(context, OcrLanguageListActivity.class)))
                .setNegativeButton(R.string.cancel, (dialogInterface, i) -> dialogInterface.dismiss())
                .create();
    }

    private androidx.appcompat.app.AlertDialog getNoSdCardDialog(Context context) {
        return new androidx.appcompat.app.AlertDialog.Builder(context)
                .setTitle(R.string.no_sd_card)
                .setPositiveButton(android.R.string.ok, (dialog, which) -> finish())
                .create();
    }


    @Override
    protected void onSaveInstanceState(Bundle savedInstanceState) {
        Log.i(LOG_TAG, "onSaveInstanceState" + this);
        super.onSaveInstanceState(savedInstanceState);
        if (dateCameraIntentStarted != null) {
            savedInstanceState.putLong(DATE_CAMERA_INTENT_STARTED_STATE, dateCameraIntentStarted.getTime());
        }
        if (cameraPicUri != null) {
            savedInstanceState.putString(CAMERA_PIC_URI_STATE, cameraPicUri.toString());
        }
        if (localCameraPicUri != null) {
            savedInstanceState.putString(CAMERA_PIC_LOCAL_URI_STATE, localCameraPicUri.toString());
        }
    }

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        Log.i(LOG_TAG, "onRestoreInstanceState " + this);
        super.onRestoreInstanceState(savedInstanceState);

        if (savedInstanceState.containsKey(DATE_CAMERA_INTENT_STARTED_STATE)) {
            dateCameraIntentStarted = new Date(savedInstanceState.getLong(DATE_CAMERA_INTENT_STARTED_STATE));
        }
        if (savedInstanceState.containsKey(CAMERA_PIC_URI_STATE)) {
            cameraPicUri = Uri.parse(savedInstanceState.getString(CAMERA_PIC_URI_STATE));
        }
        if (savedInstanceState.containsKey(CAMERA_PIC_LOCAL_URI_STATE)) {
            localCameraPicUri = Uri.parse(savedInstanceState.getString(CAMERA_PIC_LOCAL_URI_STATE));
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int itemId = item.getItemId();
        if (itemId == R.id.item_camera) {
            checkRam(MemoryWarningDialog.DoAfter.START_CAMERA);
            return true;
        } else if (itemId == R.id.item_gallery) {
            checkRam(MemoryWarningDialog.DoAfter.START_GALLERY);
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.base_document_activity_options, menu);
        return true;
    }

    private void startOcr(ArrayList<Uri> uris) {
        Intent intent = new Intent(this, OcrPdfActivity.class);
        ClipData clipData = null;
        for (Uri uri : uris) {
            if (clipData == null) {
                clipData = ClipData.newUri(getContentResolver(), "", uri);
            } else {
                clipData.addItem(new ClipData.Item(uri));
            }
        }
        intent.setClipData(clipData);
        intent.putExtra(OCRActivity.EXTRA_PARENT_DOCUMENT_ID, getParentId());
        startActivity(intent);
    }

    private boolean isOneImage(ArrayList<Uri> uris) {
        if (uris.size() != 1) {
            return false;
        }
        final Uri uri = uris.get(0);
        final String type = getContentResolver().getType(uri);
        return type != null && type.contains("image");
    }

    private Uri loadCameraResult() {
        Cursor myCursor = null;
        Date dateOfPicture;
        //check if there is a file at the uri we specified
        if (cameraPicUri != null) {
            File f = new File(localCameraPicUri.getPath());
            if (f.isFile() && f.exists() && f.canRead()) {
                //all is well
                Log.i(LOG_TAG, "onTakePhotoActivityResult");
                return localCameraPicUri;
            }

        }
        //try to look up the image by querying the media content provider
        try {
            // Create a Cursor to obtain the file Path for the large
            // image
            String[] largeFileProjection = {MediaStore.Images.ImageColumns._ID, MediaStore.Images.ImageColumns.DATA, MediaStore.Images.ImageColumns.ORIENTATION, MediaStore.Images.ImageColumns.DATE_TAKEN};
            String largeFileSort = MediaStore.Images.ImageColumns._ID + " DESC";
            myCursor = getContentResolver().query(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, largeFileProjection, null, null, largeFileSort);
            if (myCursor != null) {
                myCursor.moveToFirst();
                // This will actually give you the file path location of the
                // image.
                String largeImagePath = myCursor.getString(myCursor.getColumnIndexOrThrow(MediaStore.Images.ImageColumns.DATA));
                Uri tempCameraPicUri = Uri.fromFile(new File(largeImagePath));
                dateOfPicture = new Date(myCursor.getLong(myCursor.getColumnIndexOrThrow(MediaStore.Images.ImageColumns.DATE_TAKEN)));
                if (dateOfPicture.getTime() == 0 || (dateOfPicture.after(dateCameraIntentStarted))) {
                    return tempCameraPicUri;
                }
            }
        } catch (Exception ignored) {
        } finally {
            if (myCursor != null) {
                myCursor.close();
            }
        }
        return null;
    }

    protected void startOcr(final Uri cameraPicUri, ImageSource source) {
        mCrashLogger.logMessage("Loading " + cameraPicUri.toString() + " from " + source.name());
        Intent intent = new Intent(this, OCRActivity.class);
        intent.putExtra(EXTRA_IMAGE_SOURCE, source.name());
        intent.setData(cameraPicUri);
        intent.putExtra(OCRActivity.EXTRA_PARENT_DOCUMENT_ID, getParentId());
        startActivityForResult(intent, REQUEST_CODE_OCR);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if (RESULT_OK == resultCode) {
            switch (requestCode) {
                case REQUEST_CODE_MAKE_PHOTO:
                case REQUEST_CODE_PICK_PHOTO:
                    mCameraResult = new CameraResult(requestCode, resultCode, data);
                    break;
                case REQUEST_CODE_OCR:
                    final Intent intent = new Intent(this, DocumentActivity.class);
                    intent.putExtra(EXTRA_ACCURACY, data.getIntExtra(EXTRA_ACCURACY, 0));
                    intent.putExtra(EXTRA_LANGUAGE, data.getStringExtra(EXTRA_LANGUAGE));
                    intent.setData(data.getData());
                    intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
                    startActivity(intent);
                    break;
            }
        } else if (OCRActivity.RESULT_NEW_IMAGE == resultCode) {
            String source = data.getStringExtra(EXTRA_IMAGE_SOURCE);
            switch (ImageSource.valueOf(source)) {
                case PICK:
                    startGallery();
                    break;
                case INTENT:
                    finish();
                    break;
                case CAMERA:
                    startCamera();
                    break;
            }
        }
    }

    @Override
    protected void onResumeFragments() {
        super.onResumeFragments();
        if (mCameraResult != null) {
            final int resultCode = mCameraResult.mResultCode;
            final int requestCode = mCameraResult.mRequestCode;
            final Intent intent = mCameraResult.mData;
            mCameraResult = null;

            if (resultCode != RESULT_OK) {
                return;
            }

            if (requestCode == REQUEST_CODE_MAKE_PHOTO) {
                onImageFromCamera();
            } else if (requestCode == REQUEST_CODE_PICK_PHOTO) {
                onDataFromGallery(intent, ImageSource.PICK);
            }

        }
    }

    protected void onDataFromGallery(Intent intent, ImageSource imageSource) {
        final ArrayList<Uri> uris = new ArrayList<>();
        if (intent.getClipData() != null) {
            for (int i = 0; i < intent.getClipData().getItemCount(); i++) {
                final ClipData.Item itemAt = intent.getClipData().getItemAt(i);
                uris.add(itemAt.getUri());
            }
        }
        try {
            if (intent.getData() != null) {
                uris.add(intent.getData());
            }
        } catch (Exception ignored) {
        }
        if (uris.isEmpty()) {
            showFileError(this, PixLoadStatus.MEDIA_STORE_RETURNED_NULL);
        } else {
            startOcr(uris.get(0), imageSource);
        }
        //TODO enable SEND_MULTIPLE in manifest
//        if (isOneImage(uris)) {
//            startOcr(uris.get(0), imageSource);
//        } else {
//            startOcr(uris);
//        }
    }


    private void onImageFromCamera() {
        final Uri uri = loadCameraResult();
        if (uri == null) {
            showFileError(this, PixLoadStatus.CAMERA_NO_IMAGE_RETURNED);
        } else {
            startOcr(uri, ImageSource.CAMERA);
        }
    }


    public static void showFileError(Context context, PixLoadStatus status) {
        showFileError(context, status, null);
    }

    public static void showFileError(Context context, PixLoadStatus second, OnClickListener positiveListener) {
        int textId;
        switch (second) {
            case IMAGE_NOT_32_BIT:
                textId = R.string.image_not_32_bit;
                break;
            case IMAGE_FORMAT_UNSUPPORTED:
                textId = R.string.image_format_unsupported;
                break;
            case IMAGE_COULD_NOT_BE_READ:
                textId = R.string.image_could_not_be_read;
                break;
            case IMAGE_DOES_NOT_EXIST:
                textId = R.string.image_does_not_exist;
                break;
            case IO_ERROR:
                textId = R.string.gallery_io_error;
                break;
            case CAMERA_APP_NOT_FOUND:
                textId = R.string.camera_app_not_found;
                break;
            case MEDIA_STORE_RETURNED_NULL:
                textId = R.string.media_store_returned_null;
                break;
            case CAMERA_APP_ERROR:
                textId = R.string.camera_app_error;
                break;
            case CAMERA_NO_IMAGE_RETURNED:
                textId = R.string.camera_no_image_returned;
                break;
            default:
                textId = R.string.error_could_not_take_photo;
        }

        AlertDialog.Builder alert = new AlertDialog.Builder(context);
        alert.setTitle(R.string.error_title);
        final TextView textview = new TextView(context);
        textview.setText(textId);
        alert.setView(textview);
        alert.setPositiveButton(android.R.string.ok, positiveListener);
        alert.show();
    }

    @Override
    protected Dialog onCreateDialog(int id, Bundle args) {
        switch (id) {

            case PDF_PROGRESS_DIALOG_ID:
                int max = args.getInt(DIALOG_ARG_MAX);
                String message = args.getString(DIALOG_ARG_MESSAGE);
                String title = args.getString(DIALOG_ARG_TITLE);
                pdfProgressDialog = new ProgressDialog(this);
                pdfProgressDialog.setMessage(message);
                pdfProgressDialog.setTitle(title);
                pdfProgressDialog.setIndeterminate(false);
                pdfProgressDialog.setMax(max);
                pdfProgressDialog.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
                pdfProgressDialog.setCancelable(false);
                return pdfProgressDialog;
            case DELETE_PROGRESS_DIALOG_ID:
                max = args.getInt(DIALOG_ARG_MAX);
                message = args.getString(DIALOG_ARG_MESSAGE);
                deleteProgressDialog = new ProgressDialog(this);
                deleteProgressDialog.setMessage(message);
                deleteProgressDialog.setIndeterminate(false);
                deleteProgressDialog.setMax(max);
                deleteProgressDialog.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
                deleteProgressDialog.setCancelable(false);
                return deleteProgressDialog;
            case EDIT_TITLE_DIALOG_ID:
                View layout = getLayoutInflater().inflate(R.layout.dialog_edit_title, null);
                final Uri documentUri = Uri.parse(args.getString(DIALOG_ARG_DOCUMENT_URI));
                final String oldTitle = args.getString(DIALOG_ARG_TITLE);
                final EditText edit = (EditText) layout.findViewById(R.id.edit_title);
                edit.setText(oldTitle);

                AlertDialog.Builder builder = new Builder(this);
                builder.setView(layout);
                builder.setTitle(R.string.edit_dialog_title);
                builder.setIcon(R.drawable.fairy_showing);
                builder.setPositiveButton(android.R.string.ok, new OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        final String title = edit.getText().toString();
                        saveTitle(title, documentUri);

                    }
                });
                builder.setNegativeButton(R.string.cancel, new OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialog, int which) {

                    }
                });
                builder.show();
        }
        return super.onCreateDialog(id, args);
    }

    @Override
    protected void onPrepareDialog(int id, Dialog dialog, Bundle args) {
        switch (id) {
            case EDIT_TITLE_DIALOG_ID:
                final Uri documentUri = Uri.parse(args.getString(DIALOG_ARG_DOCUMENT_URI));
                final String oldTitle = args.getString(DIALOG_ARG_TITLE);
                final EditText edit = (EditText) dialog.findViewById(R.id.edit_title);
                edit.setText(oldTitle);
                AlertDialog alertDialog = (AlertDialog) dialog;
                Button okButton = alertDialog.getButton(AlertDialog.BUTTON_NEUTRAL);
                okButton.setOnClickListener(new View.OnClickListener() {

                    @Override
                    public void onClick(View v) {
                        final String title = edit.getText().toString();
                        saveTitle(title, documentUri);
                    }
                });
                break;
            case HINT_DIALOG_ID:
                break;
            default:
                if (args != null) {
                    final int max = args.getInt(DIALOG_ARG_MAX);
                    final int progress = args.getInt(DIALOG_ARG_PROGRESS);
                    // final int secondaryProgress =
                    // args.getInt(DIALOG_ARG_SECONDARY_PROGRESS);
                    final String message = args.getString(DIALOG_ARG_MESSAGE);
                    final String title = args.getString(DIALOG_ARG_TITLE);
                    if (id == PDF_PROGRESS_DIALOG_ID) {
                        runOnUiThread(new Runnable() {

                            @Override
                            public void run() {
                                pdfProgressDialog.setProgress(progress);
                                pdfProgressDialog.setMax(max);
                                if (message != null) {
                                    pdfProgressDialog.setMessage(message);
                                }
                                if (title != null) {
                                    pdfProgressDialog.setTitle(title);
                                }
                            }
                        });

                    } else if (id == DELETE_PROGRESS_DIALOG_ID) {
                        runOnUiThread(new Runnable() {

                            @Override
                            public void run() {
                                deleteProgressDialog.setProgress(progress);
                                deleteProgressDialog.setMax(max);
                                if (message != null) {
                                    deleteProgressDialog.setMessage(message);
                                }
                            }
                        });
                    }
                }
        }
        super.onPrepareDialog(id, dialog, args);
    }

    protected void askUserForNewTitle(final String oldTitle, final Uri documentUri) {
        Bundle bundle = new Bundle(2);
        bundle.putString(DIALOG_ARG_TITLE, oldTitle);
        bundle.putString(DIALOG_ARG_DOCUMENT_URI, documentUri.toString());
        showDialog(EDIT_TITLE_DIALOG_ID, bundle);
    }

    private void saveTitle(final String newTitle, final Uri documentUri) {
        Uri uri = documentUri;
        if (uri == null) {
            uri = getIntent().getData();
        }
        if (uri != null) {
            SaveDocumentTask saveTask = new SaveDocumentTask(this, documentUri, newTitle);
            saveTask.execute();
        }

    }


    /**
     * *******************************************
     * <p/>
     * ASYNC TASKS
     */

    protected class CreatePDFTask extends AsyncTask<Void, Integer, Pair<ArrayList<Uri>, ArrayList<Uri>>> implements PDFProgressListener {

        private Set<Integer> mIds = new HashSet<Integer>();
        private int mCurrentPageCount;
        private int mCurrentDocumentIndex;
        private String mCurrentDocumentName;
        private StringBuilder mOCRText = new StringBuilder();

        public CreatePDFTask(Set<Integer> ids) {
            mIds.addAll(ids);
        }

        @Override
        public void onNewPage(final int pageNumber) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    Bundle args = new Bundle(5);

                    String progressMsg = getResources().getString(R.string.progress_pfd_creation);
                    progressMsg = String.format(progressMsg, pageNumber, mCurrentPageCount, mCurrentDocumentName);

                    String title = getResources().getString(R.string.pdf_creation_message);

                    args.putString(DIALOG_ARG_MESSAGE, title);
                    args.putString(DIALOG_ARG_MESSAGE, progressMsg);
                    args.putInt(DIALOG_ARG_MAX, mIds.size());
                    args.putInt(DIALOG_ARG_PROGRESS, mCurrentDocumentIndex);
                    args.putInt(DIALOG_ARG_SECONDARY_PROGRESS, pageNumber);
                    showDialog(PDF_PROGRESS_DIALOG_ID, args);
                }
            });

        }

        @Override
        protected void onPreExecute() {
            mOCRText.delete(0, mOCRText.length());
            Bundle args = new Bundle(2);
            args.putInt(DIALOG_ARG_MAX, mIds.size());
            args.putInt(DIALOG_ARG_PROGRESS, 0);
            String message = getText(R.string.pdf_creation_message).toString();
            args.putString(DIALOG_ARG_MESSAGE, message);
            showDialog(PDF_PROGRESS_DIALOG_ID, args);
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Pair<ArrayList<Uri>, ArrayList<Uri>> files) {
            removeDialog(PDF_PROGRESS_DIALOG_ID);
            if (files != null && files.first.size() > 0) {
                if (files.first.size() > 1) {
                    //we have more than one pdf file
                    //share by sending them
                    sharePDFBySending(files);
                } else {
                    // single pdf file
                    // share by opening pdf viewer
                    Intent target = new Intent(Intent.ACTION_VIEW);
                    target.setDataAndType(files.first.get(0), "application/pdf");
                    target.setFlags(Intent.FLAG_ACTIVITY_NO_HISTORY);
                    target.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
                    Intent intent = Intent.createChooser(target, "Open File");
                    try {
                        startActivity(intent);
                    } catch (ActivityNotFoundException e) {
                        sharePDFBySending(files);
                    }

                }


            } else {
                Toast.makeText(getApplicationContext(), getText(R.string.error_create_file), Toast.LENGTH_LONG).show();
            }
        }

        private void sharePDFBySending(Pair<ArrayList<Uri>, ArrayList<Uri>> files) {
            Intent shareIntent = new Intent(android.content.Intent.ACTION_SEND_MULTIPLE);
            shareIntent.putExtra(android.content.Intent.EXTRA_SUBJECT, getText(R.string.share_subject));
            CharSequence seq = Html.fromHtml(mOCRText.toString());
            shareIntent.putExtra(android.content.Intent.EXTRA_TEXT, seq);
            shareIntent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
            shareIntent.setType("application/pdf");
            ArrayList<Uri> allFiles = new ArrayList<Uri>();
            allFiles.addAll(files.first);
            allFiles.addAll(files.second);
            shareIntent.putParcelableArrayListExtra(Intent.EXTRA_STREAM, allFiles);
            startActivity(Intent.createChooser(shareIntent, getText(R.string.share_chooser_title)));
        }

        private Pair<File, File> createPDF(File dir, long documentId) {

            Cursor cursor = getContentResolver().query(DocumentContentProvider.CONTENT_URI, null, Columns.PARENT_ID + "=? OR " + Columns.ID + "=?",
                    new String[]{String.valueOf(documentId), String.valueOf(documentId)}, "created ASC");
            cursor.moveToFirst();

            int index = cursor.getColumnIndex(Columns.TITLE);
            final String fileName = documentId + ".pdf";
            File outPdf = new File(dir, fileName);
            File outText = new File(dir, documentId + ".txt");

            mCurrentDocumentName = fileName;
            mCurrentPageCount = cursor.getCount();
            String[] images = new String[cursor.getCount()];
            String[] hocr = new String[cursor.getCount()];
            cursor.moveToPosition(-1);
            boolean overlayImage = true;
            while (cursor.moveToNext()) {
                int hocrIndex = cursor.getColumnIndex(Columns.HOCR_TEXT);
                index = cursor.getColumnIndex(Columns.PHOTO_PATH);
                final String photoPath = cursor.getString(index);
                if (photoPath != null) {
                    Uri imageUri = Uri.parse(photoPath);
                    images[cursor.getPosition()] = Util.getPathForUri(NewDocumentActivity.this, imageUri);
                } else {
                    overlayImage = false;
                }
                index = cursor.getColumnIndex(Columns.OCR_TEXT);
                final String text = cursor.getString(index);
                if (text != null && text.length() > 0) {
                    hocr[cursor.getPosition()] = cursor.getString(hocrIndex);
                    FileWriter writer;
                    try {
                        writer = new FileWriter(outText);
                        final String s = Html.fromHtml(text).toString();
                        writer.write(s);
                        writer.close();
                    } catch (IOException ioException) {
                        if (outText.exists()) {
                            outText.delete();
                        }
                        outText = null;
                    }

                    mOCRText.append(text);
                } else {
                    hocr[cursor.getPosition()] = "";
                }
            }
            cursor.close();
            Hocr2Pdf pdf = new Hocr2Pdf(this);
            pdf.hocr2pdf(images, hocr, outPdf.getPath(), true, overlayImage);
            return new Pair<>(outPdf, outText);
        }

        @Override
        protected Pair<ArrayList<Uri>, ArrayList<Uri>> doInBackground(Void... params) {
            File dir = AppStorage.getPDFDir(getApplicationContext());
            if (dir == null) {
                return null;
            }
            if (!dir.exists()) {
                if (!dir.mkdir()) {
                    return null;
                }
            }

            ArrayList<Uri> pdfFiles = new ArrayList<>();
            ArrayList<Uri> txtFiles = new ArrayList<>();
            mCurrentDocumentIndex = 0;
            for (long id : mIds) {
                final Pair<File, File> pair = createPDF(dir, id);
                final File pdf = pair.first;
                final File text = pair.second;
                if (pdf != null) {
                    pdfFiles.add(toUri(pdf));
                }
                if (text != null) {
                    txtFiles.add(toUri(text));
                }
                mCurrentDocumentIndex++;
            }
            return Pair.create(pdfFiles, txtFiles);
        }
    }

    private Uri toUri(File file) {
        return FileProvider.getUriForFile(
                getApplicationContext(),
                getString(R.string.config_share_file_auth),
                file
        );
    }

    protected class DeleteDocumentTask extends AsyncTask<Void, Void, Integer> {
        Set<Integer> mIds = new HashSet<Integer>();
        private final static int RESULT_REMOTE_EXCEPTION = -1;
        final boolean mFinishActivity;

        public DeleteDocumentTask(Set<Integer> parentDocumentIds, final boolean finishActivityAfterExecution) {
            mIds.addAll(parentDocumentIds);
            mFinishActivity = finishActivityAfterExecution;
        }

        @Override
        protected void onPreExecute() {
            Bundle args = new Bundle(2);
            args.putInt(DIALOG_ARG_MAX, mIds.size());
            String message = getText(R.string.delete_dialog_message).toString();
            args.putString(DIALOG_ARG_MESSAGE, message);
            showDialog(DELETE_PROGRESS_DIALOG_ID, args);
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(Integer result) {
            if (result == RESULT_REMOTE_EXCEPTION) {
                Toast.makeText(getApplicationContext(), getText(R.string.delete_error), Toast.LENGTH_LONG).show();
            }
            removeDialog(DELETE_PROGRESS_DIALOG_ID);
            super.onPostExecute(result);
            if (mFinishActivity) {
                finish();
            }
        }

        private int deleteDocument(Cursor c, ContentProviderClient client) throws RemoteException {
            int index = c.getColumnIndex(Columns.ID);
            int currentId = c.getInt(index);
            Uri currentDocumentUri = Uri.withAppendedPath(DocumentContentProvider.CONTENT_URI, String.valueOf(currentId));
            index = c.getColumnIndex(Columns.PHOTO_PATH);
            String imagePath = c.getString(index);
            if (imagePath != null) {
                new File(imagePath).delete();
            }
            return client.delete(currentDocumentUri, null, null);
        }

        @Override
        protected Integer doInBackground(Void... params) {

            ContentProviderClient client = getContentResolver().acquireContentProviderClient(DocumentContentProvider.CONTENT_URI);

            int count = 0;
            int progress = 0;
            for (Integer id : mIds) {
                try {
                    Cursor c = client.query(DocumentContentProvider.CONTENT_URI, new String[]{Columns.ID, Columns.PHOTO_PATH}, Columns.PARENT_ID + "=? OR " + Columns.ID + "=?",
                            new String[]{String.valueOf(id), String.valueOf(id)}, Columns.PARENT_ID + " ASC");

                    while (c.moveToNext()) {
                        count += deleteDocument(c, client);
                    }

                } catch (RemoteException exc) {
                    return RESULT_REMOTE_EXCEPTION;
                }
                deleteProgressDialog.setProgress(++progress);
            }
            return count;
        }
    }

    public static class SaveDocumentTask extends AsyncTask<Void, Integer, Integer> {

        private final Context mContext;
        private ContentValues values = new ContentValues();
        private ArrayList<Uri> mDocumentUri = new ArrayList<>();
        private String mTitle;
        private ArrayList<Spanned> mOcrText = new ArrayList<>();
        private Toast mSaveToast;

        public SaveDocumentTask(Context context, List<Uri> documentUri, List<Spanned> ocrText) {
            mContext = context;
            this.mDocumentUri.addAll(documentUri);
            this.mTitle = null;
            this.mOcrText.addAll(ocrText);
        }

        public SaveDocumentTask(Context context, Uri documentUri, String title) {
            mContext = context;
            this.mDocumentUri.add(documentUri);
            this.mTitle = title;
        }

        @Override
        protected void onPreExecute() {
            mSaveToast = Toast.makeText(mContext, mContext.getText(R.string.saving_document), Toast.LENGTH_LONG);
        }

        @Override
        protected void onPostExecute(Integer result) {
            if (result != null && result > 0) {
                mSaveToast.setText(R.string.save_success);
            } else {
                mSaveToast.setText(R.string.save_fail);
            }
        }

        @Override
        protected Integer doInBackground(Void... params) {
            mSaveToast.show();
            int result = 0;
            for (int i = 0; i < mDocumentUri.size(); i++) {
                values.clear();
                Uri uri = mDocumentUri.get(i);
                if (mOcrText != null && i < mOcrText.size()) {
                    final String text = Html.toHtml(mOcrText.get(i));
                    values.put(Columns.OCR_TEXT, text);

                }
                if (mTitle != null) {
                    values.put(Columns.TITLE, mTitle);
                }
                publishProgress(i);
                result += mContext.getContentResolver().update(uri, values, null, null);
            }
            return result;
        }
    }


}
