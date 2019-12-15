package com.locii.docuscanlib

abstract class DocuScan {

    private val nativeObject = createDocuScan()


    /**
     * a method called to process an image - scan it for a document
     * return an address of a cropped, transformed document
     *
     * @param matAddrIn the native address of the Mat to process
     * @param nativeObject the native address of a DocuScan object allocated with createDocuScan
     * @param extraAddr1 the native address of a mat to hold intermediate/temp results
     * @param extraAddr2 the native address of a mat to hold intermediate/temp results
     * @return the address of the cropped Mat of the result. or -1 if did not meet the requirements
     *
     */
    private external fun scanDocument(
        matAddrIn: Long,
        nativeObject: Long,
        extraAddr1: Long,
        extraAddr2: Long
    ): Long


    // create a new native object
     external fun createDocuScan(): Long

    // release an object allocated with createDocuScan
     external fun releaseDocuScan(addr: Long)


    fun scan(
        matAddrIn: Long,
        extraAddr1: Long,
        extraAddr2: Long
    ): Long {
        return scanDocument(nativeObject,matAddrIn,extraAddr1,extraAddr2)
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