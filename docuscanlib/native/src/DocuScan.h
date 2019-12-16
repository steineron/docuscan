//
// Created by Ron Steiner on 2019-12-15.
//

#include <jni.h>
#include "opencv2/core/types.hpp"


#ifndef DOCUSCAcreateDocuScan_H
#define DOCUSCAcreateDocuScan_H

using namespace cv;

extern "C" {
JNIEXPORT jlong
JNICALL
Java_com_locii_docuscanlib_DocuScan_scanDocument(JNIEnv *, jobject, jlong nativeObject,
                                                 jlong addrTrgtMat, jlong temp1Addr,
                                                 jlong temp2Addr);

JNIEXPORT jlong
JNICALL
Java_com_locii_docuscanlib_DocuScan_createDocuScan(JNIEnv *, jobject);


JNIEXPORT void
JNICALL
Java_com_locii_docuscanlib_DocuScan_releaseDocuScan(JNIEnv *, jobject, jlong nativeObject);

JNIEXPORT void
JNICALL
Java_com_locii_docuscanlib_DocuScan_setGuide(JNIEnv *, jobject, jlong nativeObject, jfloat tl_x,
                                             jfloat tl_y, jfloat br_x, jfloat br_y);

JNIEXPORT void
JNICALL
Java_com_locii_docuscanlib_DocuScan_setSharpness(JNIEnv *, jobject, jlong nativeObject,
                                                 jint sharpness);


JNIEXPORT void
JNICALL
Java_com_locii_docuscanlib_DocuScan_setDistance(JNIEnv *, jobject, jlong nativeObject,
                                                jint distance);

class DocuScan;

}
#endif //DOCUSCAcreateDocuScan_H
