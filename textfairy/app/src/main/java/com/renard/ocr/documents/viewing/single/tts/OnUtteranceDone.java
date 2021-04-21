package com.renard.ocr.documents.viewing.single.tts;

class OnUtteranceDone {
    private final String mUtteranceId;

    public OnUtteranceDone(String utteranceId) {

        mUtteranceId = utteranceId;
    }

    public String getUtteranceId() {
        return mUtteranceId;
    }
}
