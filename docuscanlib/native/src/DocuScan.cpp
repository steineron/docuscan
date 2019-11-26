//
// Created by Ron Steiner on 2019-11-26.
//

#include <jni.h>
#include <string>
#include <iostream>
#include "opencv2/core/utility.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"

using namespace std;
using namespace cv;

extern "C" JNIEXPORT jstring JNICALL
Java_com_locii_docuscanlib_DocuScan_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}




/**
 * find the closesest point to @param p in @param points by computing
 * the minimum Euclidean Distance between the point p and any of the other points
 * @param p
 * @param points
 * @return the point in points closest to p
 */
static Point2f closestPoint(Point &p, Point2f points[], int nPoints){

    int minDistance = INT_MAX;
    Point2f result;
    for(int i=0; i<nPoints; i++){
        Point2f candidate = points[i];
        int distance = pow(candidate.x - p.x, 2) + pow(candidate.y - p.y, 2);
        if(distance < minDistance){
            minDistance=distance;
            result=candidate;
        }
    }

    return result;
}
