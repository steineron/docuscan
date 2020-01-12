package com.locii.docuscanlib

import android.graphics.RectF

abstract class DocuScan {

    private val nativeObject = createDocuScan()


    /**
     * a method called to process an image - scan it for a document
     * return an address of a cropped, transformed document
     *
     * @param nativeObject the native address of a DocuScan object allocated with createDocuScan
     * @param matAddrIn the native address of the Mat to process
     * @param extraAddr1 the native address of a mat to hold intermediate/temp results
     * @param extraAddr2 the native address of a mat to hold intermediate/temp results
     * @return the address of the cropped Mat of the result. or -1 if did not meet the requirements
     *
     */
    private external fun scanDocument(
        nativeObject: Long,
        matAddrIn: Long,
        extraAddr1: Long,
        extraAddr2: Long
    ): Long


    // create a new native object
    private external fun createDocuScan(): Long

    // release an object allocated with createDocuScan
    private external fun releaseDocuScan(addr: Long)

    // set distance value
    var distance: Int = 0
        set(value) {
            field = value
            setDistance(nativeObject, value)
        }

    private external fun setDistance(addr: Long, distance: Int)

    var devMode:Boolean = false
    set(value) {
        field = value
        setDevMode(nativeObject,value)
    }

    private external fun setDevMode(addr: Long, devModeOn: Boolean)


    // set distance value
    var sharpness: Int = 0
        set(value) {
            field = value
            setSharpness(nativeObject, value)
        }

    private external fun setSharpness(addr: Long, sharpness: Int)

    var guidingRect: RectF? = null
    set(value) {
        field=value
        setGuide(nativeObject,
            value?.left ?: 0.0f,
            value?.top ?: 0.0f,
            value?.right ?: 0.0f,
            value?.bottom ?: 0.0f
            )
    }

    private external fun setGuide(addr: Long, tl_x: Float, tl_y: Float, br_x: Float, br_y: Float)



    fun scan(
        matAddrIn: Long,
        extraAddr1: Long,
        extraAddr2: Long
    ): Long {
        return scanDocument(nativeObject, matAddrIn, extraAddr1, extraAddr2)
    }

    fun release() {
        releaseDocuScan(nativeObject)
    }

    abstract fun onIntermitentMat(matAddrOut: Long)

    abstract fun onResultMat(matAddrOut: Long)

    companion object {

        // Used to load the 'native-lib' library on application startup.
        init {
            System.loadLibrary("docuscan_jni_shared")
        }
    }
}