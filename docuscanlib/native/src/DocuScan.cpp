//
// Created by Ron Steiner on 2019-11-26.
//

#include <jni.h>
#include <android/log.h>
#include <string>
#include <iostream>
#include "opencv2/core/utility.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"

using namespace std;
using namespace cv;

static Mat *processImage(Mat &srcImage, Mat &targetImage, Mat &contouredImage1, Mat &contouredImage2);

static void logMeanAndStd(Mat &mean, Mat &stdDev);

static void logOStream(ostringstream &ios);

void dilateAndErode(const Mat &mat);

extern "C" JNIEXPORT jstring JNICALL
Java_com_locii_docuscanlib_DocuScan_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C" {
JNIEXPORT jlong JNICALL
Java_com_locii_docuscanlib_DocuScan_scanDocument(JNIEnv *, jobject, jlong addrSrcMat,
                                                 jlong addrTrgtMat,jlong temp1Addr,jlong temp2Addr);


JNIEXPORT jlong JNICALL
Java_com_locii_docuscanlib_DocuScan_scanDocument(JNIEnv *env, jobject thiz, jlong addrSrcMat,
                                                 jlong addrTrgtMat,jlong temp1Addr,jlong temp2Addr) {
    Mat &src = *(Mat *) addrSrcMat;
    Mat &trgt = *(Mat *) addrSrcMat;

    Mat &contouredImage1 = *(Mat *) temp1Addr;
    Mat &contouredImage2 = *(Mat *) temp2Addr;

    jclass pJclass = (env)->GetObjectClass(thiz);

    Mat *processed = processImage(src, trgt, contouredImage1, contouredImage2);

    if (processed != NULL) {


        jmethodID midCallback = (env)->GetMethodID(pJclass, "onIntermitentMat", "(J)V");
        (env)->CallVoidMethod(thiz, midCallback, (jlong) &contouredImage1);
        (env)->CallVoidMethod(thiz, midCallback, (jlong) &contouredImage2);

        jmethodID mResultCallback = (env)->GetMethodID(pJclass, "onResultMat", "(J)V");

        (env)->CallVoidMethod(thiz, mResultCallback, (jlong) processed);

        return 0;
    }
    return -1;
}

}


/**
 * find the closesest point to @param p in @param points by computing
 * the minimum Euclidean Distance between the point p and any of the other points
 * @param p
 * @param points
 * @return the point in points closest to p
 */
static Point2f closestPoint(Point &p, Point2f points[], int nPoints) {

    int minDistance = INT_MAX;
    Point2f result;
    for (int i = 0; i < nPoints; i++) {
        Point2f candidate = points[i];
        int distance = pow(candidate.x - p.x, 2) + pow(candidate.y - p.y, 2);
        if (distance < minDistance) {
            minDistance = distance;
            result = candidate;
        }
    }

    return result;
}

#define APPNAME "LOCII_NATIVE"

static void logcat(const char *msg) {
    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "%s", msg);
}


static void logOStream(ostringstream &ios) {
    logcat(ios.str().data());
    ios.str("");
    ios.clear();
}

static void logMeanAndStd(Mat &mean, Mat &stdDev) {
    ostringstream msg;
    msg << "Sharpness: " << mean << " " << stdDev;
    logOStream(msg);


    msg << "Mean: " << mean.at<double>(0, 0) << ", " << mean.at<double>(0, 1) << ", "
        << mean.at<double>(0, 2) << ", " << mean.at<double>(0, 3);
    logOStream(msg);


    msg << "Std: " << stdDev.at<double>(0, 0) << ", " << stdDev.at<double>(0, 1) << ", "
        << stdDev.at<double>(0, 2) << ", " << stdDev.at<double>(0, 3);
    logOStream(msg);

}

static Mat *processImage(Mat &srcImage, Mat &mat, Mat &contouredImage1, Mat &contouredImage2) {
    Mat gray, blurImage, edge1;
    ostringstream msg;


    // create a BW image:
    cvtColor(srcImage, gray, COLOR_BGR2GRAY);


    Mat lap;
    Mat mean(1, 4, CV_64F), stdDev(1, 4, CV_64F);

//    compute sharpness as a standard deviation of the Laplacian
    Laplacian(gray, lap, CV_64F);
    meanStdDev(lap, mean, stdDev);

    logMeanAndStd(mean, stdDev);

    // require decent degree of sharpness
    if (stdDev.at<double>(0, 0) < 4.0) {
        return NULL;
    }


    // blur it to educe noise
    blur(gray, blurImage, Size(3, 3));
    gray.release();



    // convert to binary image
    Mat binary;
    binary.create(blurImage.size(), CV_8UC1);

    threshold(blurImage, binary, 25, 255, THRESH_BINARY_INV + THRESH_OTSU);
    blurImage.release();


    if (binary.type() != CV_8UC1) {
        binary.convertTo(binary, CV_8UC1);
    }


    // dilate, erode, dilate to further remove noise and small objects
    dilateAndErode(binary);

    // Run the edge detector on grayscale
    // Canny recommended a upper:lower ratio between 2:1 and 3:1 (see https://docs.opencv.org/3.4/da/d5c/tutorial_canny_detector.html)
    Canny(binary, edge1, 1, 3, 3);
    binary.release();

    // find contours in hte canny image (edged)
    int levels = 3;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    vector<vector<Point> > contours0;
    findContours(edge1, contours0, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);


    edge1.release();

    contours.resize(contours0.size());
    for (size_t k = 0; k < contours0.size(); k++)
        approxPolyDP(Mat(contours0[k]), contours[k], 3, true);

    // see example in https://docs.opencv.org/master/db/d00/samples_2cpp_2squares_8cpp-example.html#a17
    // for extracting squares

    // or do this: find the contour with largest area - assume it is the document

    int _levels = levels - 3;
    int maxArea = 0, max = 0;
    vector<Point> approxPoly;
    for (int i = 0; i < contours.size(); i++) {
        int current = contourArea(contours[i], false);

        if (current > max) {
            // check the shape of it:
            vector<Point> approx;
            approxPolyDP(contours[i], approx, arcLength(contours[i], true) * 0.02, true);

            // square contours should have 4 vertices after approximation
            // relatively large area (to filter out noisy contours)
            // and be convex.
            // Note: absolute value of an area is used because
            if (approx.size() == 4 && isContourConvex(approx)) {
                max = current;
                maxArea = i;
                approxPoly = approx; // keep record of hte approximated polygon
            }
        }
    }

    msg << "Approx Poly Size " << approxPoly.size() << ", area: " << max << ", Image size:"
        << srcImage.cols * srcImage.rows;
    logOStream(msg);

    if (approxPoly.size() != 4 || max < 0.6 * 0.6 * srcImage.cols * srcImage.rows) {
        return NULL;
    }

    /*Mat contouredImage1 = srcImage.clone();
    Mat contouredImage2 = srcImage.clone();*/
    drawContours(contouredImage1, contours, maxArea, Scalar(128, 255, 255),
                 3, LINE_AA, hierarchy, std::abs(_levels));


    // find the rotated rect of the contour (rotated = tilted/skwed)
    RotatedRect rect = minAreaRect(contours[maxArea]);
    Rect boundRect = boundingRect(contours[maxArea]);

    Point2f box[4];
    rect.points(box);
    Scalar blue = Scalar(255, 128, 0);
    Scalar red = Scalar(0, 128, 255);
    Scalar black = Scalar(0, 0, 0);
    Scalar orange = Scalar(0, 255, 220);

    String numbers[] = {"1", "2", "3", "4"};

    double textScale = 5;
    int textThinkness = 4;

    for (int j = 0; j < 4; j++) {
        line(contouredImage1, approxPoly[j], approxPoly[(j + 1) % 4], blue, 3, LINE_AA);
        putText(contouredImage1, numbers[j], approxPoly[j], FONT_HERSHEY_SIMPLEX, textScale, blue,
                textThinkness);
        line(contouredImage1, box[j], box[(j + 1) % 4], red, 3, LINE_AA);
        putText(contouredImage1, numbers[j], box[j], FONT_HERSHEY_SIMPLEX, textScale, red,
                textThinkness);
    }
    // draw the bounding rect
    rectangle(contouredImage1, Point(boundRect.x, boundRect.y),
              Point(boundRect.x + boundRect.width, boundRect.y + boundRect.height), black, 3,
              LINE_AA);


//    imshow("contours", contouredImage);
//    waitKey(0);

    // transform the skewed image
    std::vector<Point2f> polyPoints;
    std::vector<Point2f> boxPoints;
    std::vector<Point2f> boundingRectPoints;
    // poly to box:

    for (int i = 0; i < 4; i++) {
        polyPoints.push_back(Point2f(approxPoly[i].x, approxPoly[i].y));
        boundingRectPoints.push_back(closestPoint(approxPoly[i], box, 4));
    }

    // calcuate hte distance from the bounding rectangle to he polygon - smaller distance <-> less skwed image
    double distance = 0;
    for (int j = 0; j < 4; j++) {
        putText(contouredImage1, numbers[j], boundingRectPoints[j], FONT_HERSHEY_SIMPLEX, textScale,
                black, textThinkness, LINE_8);
        line(contouredImage1, boundingRectPoints[j], polyPoints[j], orange, 3, LINE_AA);

        distance += pow(boundingRectPoints[j].x - polyPoints[j].x, 2) +
                    pow(boundingRectPoints[j].y - polyPoints[j].y, 2);
    }

    msg << "Distance: " << distance;
    logOStream(msg);

    if (distance > 30000.0) {
        return NULL;
    }

    Mat transmtx = getPerspectiveTransform(polyPoints, boundingRectPoints);
    Mat transformed = Mat::zeros(srcImage.rows, srcImage.cols, CV_8UC3);
    warpPerspective(srcImage, transformed, transmtx, srcImage.size());

    // box to rect:

    polyPoints.clear();
    boundingRectPoints.clear();


    float h = rect.size.height;//MIN(rect.size.height,rect.size.width);
    float w = rect.size.width;//MAX(rect.size.height,rect.size.width);
    boundingRectPoints.push_back(Point2f(boundRect.x, boundRect.y));
    boundingRectPoints.push_back(Point2f(boundRect.x, boundRect.y + h));
    boundingRectPoints.push_back(Point2f(boundRect.x + w, boundRect.y));
    boundingRectPoints.push_back(Point2f(boundRect.x + w, boundRect.y + h));

    for (int i = 0; i < 4; ++i) {
        Point p = Point(boundingRectPoints[i].x, boundingRectPoints[i].y);
        polyPoints.push_back(closestPoint(p, box, 4));

    }



    for (int j = 0; j < 4; j++) {
        line(contouredImage2, polyPoints[j], polyPoints[(j + 1) % 4], red, 3, LINE_AA);
        putText(contouredImage2, numbers[j], polyPoints[j], FONT_HERSHEY_SIMPLEX, textScale, red,
                textThinkness);
        line(contouredImage2, boundingRectPoints[j], boundingRectPoints[(j + 1) % 4], orange, 3,
             LINE_AA);
        putText(contouredImage2, numbers[j], boundingRectPoints[j], FONT_HERSHEY_SIMPLEX, textScale,
                orange, textThinkness);
        line(contouredImage2, boundingRectPoints[j], polyPoints[j], blue, 3, LINE_AA);
    }


    // transform and crop
    transmtx = getPerspectiveTransform(polyPoints, boundingRectPoints);
    warpPerspective(transformed, transformed, transmtx, transformed.size());

    transformed.copyTo(mat);
//    contouredImage1.copyTo(temp1);
//    contouredImage2.copyTo(temp2);
    return &mat;


}

void dilateAndErode(const Mat &mat) {
    int niters = 3; // in 3 iterations

    dilate(mat, mat, Mat(), Point(-1, -1), niters);
    erode(mat, mat, Mat(), Point(-1, -1), niters * 2);
    dilate(mat, mat, Mat(), Point(-1, -1), niters);
}
