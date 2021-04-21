/*
 * Copyright (C) 2010 Google Inc.
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

package com.googlecode.leptonica.android;

/**
 * Wrapper for Leptonica's native BOXA.
 *
 * @author renard
 */
public class Boxa {

    /**
     * A pointer to the native Box object. This is used internally by native
     * code.
     */
    private final long mNativeBoxa;

    private boolean mRecycled = false;

    /**
     * Creates a new Box wrapper for the specified native BOX.
     *
     * @param nativeBox A pointer to the native BOX.
     */
    public Boxa(long nativeBoxa) {
        mNativeBoxa = nativeBoxa;
        mRecycled = false;
    }

    public int getCount() {
        return nativeGetCount(mNativeBoxa);
    }

    /**
     * Returns an array containing the coordinates of this box. See INDEX_*
     * constants for indices.
     *
     * @return an array of box oordinates
     */
    public int[] getGeometry(int index) {
        int[] geometry = new int[4];

        if (getGeometry(index, geometry)) {
            return geometry;
        }

        return null;
    }


    /**
     * Fills an array containing the coordinates of this box. See INDEX_*
     * constants for indices.
     *
     * @param geometry A 4+ element integer array to fill with coordinates.
     * @return <code>true</code> on success
     */
    public boolean getGeometry(int index, int[] geometry) {
        if (geometry.length < 4) {
            throw new IllegalArgumentException("Geometry array must be at least 4 elements long");
        }

        return nativeGetGeometry(mNativeBoxa, index, geometry);
    }

    /**
     * Releases resources and frees any memory associated with this Box.
     */
    public void recycle() {
        if (!mRecycled) {
            nativeDestroy(mNativeBoxa);

            mRecycled = true;
        }
    }


    // ***************
    // * NATIVE CODE *
    // ***************

    private static native void nativeDestroy(long nativeBox);

    private static native boolean nativeGetGeometry(long nativeBoxa, int index, int[] geometry);

    private static native int nativeGetCount(long nativeBoxa);
}
