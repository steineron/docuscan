//
// Created by Ron Steiner on 2019-11-26.
//

#include <jni.h>
#include <string>


extern "C" JNIEXPORT jstring JNICALL
Java_com_locii_docuscanlib_DocuScan_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}


