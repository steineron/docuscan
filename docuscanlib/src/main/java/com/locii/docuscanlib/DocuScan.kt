package com.locii.docuscanlib

class DocuScan{

    /**
     * A native method that is implemented by the 'opencv_jni_shared' native library,
     * which is packaged with this module
     *calls Java_com_locii_docuscanlib_DocuScan_stringFromJNI.
     */
    external fun stringFromJNI(): String


    companion object {

        // Used to load the 'native-lib' library on application startup.
        init {
            System.loadLibrary("opencv_jni_shared")
        }
    }
}