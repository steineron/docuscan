package com.locii.docuscanlib

abstract class DocuScan {

    /**
     * A native method that is implemented by the 'docuscan_jni_shared' native library,
     * which is packaged with this module
     *calls Java_com_locii_docuscanlib_DocuScan_stringFromJNI.
     */
    external fun stringFromJNI(): String

    /**
     * a method called to process an image - scan it for a document
     * return an address of a cropped, transformed document
     *
     * @param matAddrIn the native address of the Mat to process
     * @param resultAddrOut the native address of the result Mat (where the transformed image should be)
     * @param extraAddr1 the native address of a mat to hold intermediate/temp results
     * @param extraAddr2 the native address of a mat to hold intermediate/temp results
     * @return the address of the cropped Mat of the result. or -1 if did not meet the requirements
     *
     */
    external fun scanDocument(
        matAddrIn: Long,
        resultAddrOut: Long,
        extraAddr1: Long,
        extraAddr2: Long
    ): Long

    abstract fun onIntermitentMat(matAddrOut: Long)

    abstract fun onResultMat(matAddrOut: Long)

    companion object {

        // Used to load the 'native-lib' library on application startup.
        init {
            System.loadLibrary("docuscan_jni_shared")
        }
    }
}