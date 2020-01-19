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
#include "DocuScan.h"

using namespace std;
using namespace cv;


class DocuScan {

private:
    Point2f topLeft, bottomRight;
    int distance = 50000;
    double sharpness = 2.0;
    bool devMode = false;
    int linesThreshold = 50;
    int lineMinWidth = 200;
    int maxLineGap = 10;
    int edges = 3;
    int gutterSize=50;
public:

    DocuScan() : topLeft(Point2f()), bottomRight() {}

    const Point2f &getTopLeft() const {
        return topLeft;
    }

    const Point2f &getBottomRight() const {
        return bottomRight;
    }

    int getDistance() const {
        return distance;
    }

    double getSharpness() const {
        return sharpness;
    }

    bool getDevMode() const {
        return devMode;
    }

    int getLinesThreshold() const {
        return linesThreshold;
    }

    void setLinesThreshold(int linesThreshold) {
        DocuScan::linesThreshold = linesThreshold;
    }

    int getLineMinWidth() const {
        return lineMinWidth;
    }

    void setLineMinWidth(int lineMinWidth) {
        DocuScan::lineMinWidth = lineMinWidth;
    }

    int getMaxLineGap() const {
        return maxLineGap;
    }

    void setMaxLineGap(int maxLineGap) {
        DocuScan::maxLineGap = maxLineGap;
    }

    int getEdges() const {
        return edges;
    }

    void setEdges(int edges) {
        DocuScan::edges = edges;
    }

    int getGutterSize() const {
        return gutterSize;
    }

    void setGutterSize(int gutterSize) {
        DocuScan::gutterSize = gutterSize;
    }

    void setDistance(int d) {
        distance = d;
    }

    void setSharpness(int s) {
        sharpness = static_cast<double>(s);
    }

    void setGuide(Point2f &tl, Point2f &br) {
        topLeft = Point2f(tl);
        bottomRight = Point2f(br);
    }

    void setDevMode(bool on) {
        devMode = on;
    }
};

static Mat *processImage(Mat &srcImage, Mat &mat, Mat &contouredImage1, Mat &contouredImage2,
                         DocuScan &scanParams);

static Mat *extractWithContours(const Mat &srcImage, Mat &mat, const Mat &contouredImage1,
                                const Mat &contouredImage2, const DocuScan &scanParams, Mat &edged);

static Mat *extractWithHoughLines(const Mat &srcImage, Mat &mat, const Mat &devMat1,
                                  const Mat &devMat2, const DocuScan &scanParams,
                                  Mat &edged);

static void logMeanAndStd(Mat &mean, Mat &stdDev);

static void logOStream(ostringstream &ios);

void dilateAndErode(const Mat &mat);

static int euclideanDistance(const Point &p1, const Point &p2);

Mat *extractWithCountours(const Mat &srcImage, Mat &mat, const Mat &contouredImage1,
                          const Mat &contouredImage2, const DocuScan &scanParams, Mat &edge1,
                          ostringstream &msg);

extern "C" {


JNIEXPORT jlong JNICALL
Java_com_locii_docuscanlib_DocuScan_scanDocument(JNIEnv *env, jobject thiz, jlong nativeObject,
                                                 jlong addrSrcMat, jlong temp1Addr,
                                                 jlong temp2Addr) {
    Mat &src = *(Mat *) addrSrcMat;
    Mat &trgt = *(Mat *) addrSrcMat;
    DocuScan &params = *(DocuScan *) nativeObject;

    Mat &contouredImage1 = *(Mat *) temp1Addr;
    Mat &contouredImage2 = *(Mat *) temp2Addr;


    Mat *processed = processImage(src, trgt, contouredImage1, contouredImage2, params);

    if (processed != NULL) {

        jclass pJclass = (env)->GetObjectClass(thiz);

        if (params.getDevMode()) {
            jmethodID midCallback = (env)->GetMethodID(pJclass, "onIntermitentMat", "(J)V");
            (env)->CallVoidMethod(thiz, midCallback, (jlong) &contouredImage1);
            (env)->CallVoidMethod(thiz, midCallback, (jlong) &contouredImage2);
        }

        ostringstream msg;
        msg << "Result: Sharpness: " << params.getSharpness() << ", Distance: "
            << params.getDistance();
        logOStream(msg);
        jmethodID mResultCallback = (env)->GetMethodID(pJclass, "onResultMat", "(J)V");

        (env)->CallVoidMethod(thiz, mResultCallback, (jlong) processed);

        return 0;
    }
    return -1;
}


JNIEXPORT jlong JNICALL
Java_com_locii_docuscanlib_DocuScan_createDocuScan(JNIEnv *, jobject) {
    return (jlong) new DocuScan();
}

JNIEXPORT void
JNICALL
Java_com_locii_docuscanlib_DocuScan_releaseDocuScan(JNIEnv *, jobject, jlong nativeObject) {

    delete (DocuScan *) nativeObject;
}

JNIEXPORT void
JNICALL
Java_com_locii_docuscanlib_DocuScan_setDistance(JNIEnv *, jobject, jlong nativeObject,
                                                jint distance) {

    DocuScan &sd = *(DocuScan *) nativeObject;
    sd.setDistance(distance);
}
JNIEXPORT void
JNICALL
Java_com_locii_docuscanlib_DocuScan_setDevMode(JNIEnv *, jobject, jlong nativeObject,
                                               jboolean devModeOn) {

    DocuScan &sd = *(DocuScan *) nativeObject;
    sd.setDevMode(devModeOn);
}

JNIEXPORT void
JNICALL
Java_com_locii_docuscanlib_DocuScan_setSharpness(JNIEnv *, jobject, jlong nativeObject,
                                                 jint sharpness) {

    DocuScan &sd = *(DocuScan *) nativeObject;
    sd.setSharpness(sharpness);
}

JNIEXPORT void
JNICALL
Java_com_locii_docuscanlib_DocuScan_setLinesThreshold(JNIEnv *, jobject, jlong nativeObject,
                                                      jint threshold) {

    DocuScan &sd = *(DocuScan *) nativeObject;
    sd.setLinesThreshold(threshold);
}

JNIEXPORT void
JNICALL
Java_com_locii_docuscanlib_DocuScan_setMinLineLength(JNIEnv *, jobject, jlong nativeObject,
                                                     jint length) {

    DocuScan &sd = *(DocuScan *) nativeObject;
    sd.setLineMinWidth(length);
}

JNIEXPORT void
JNICALL
Java_com_locii_docuscanlib_DocuScan_setMaxLineGap(JNIEnv *, jobject, jlong nativeObject,
                                                  jint gap) {

    DocuScan &sd = *(DocuScan *) nativeObject;
    sd.setMaxLineGap(gap);
}

JNIEXPORT void
JNICALL
Java_com_locii_docuscanlib_DocuScan_setNumberOfEdges(JNIEnv *, jobject, jlong nativeObject,
                                                     jint n) {

    DocuScan &sd = *(DocuScan *) nativeObject;
    sd.setEdges(n);
}
JNIEXPORT void
JNICALL
Java_com_locii_docuscanlib_DocuScan_setGutterSize(JNIEnv *, jobject, jlong nativeObject,
                                                     jint n) {

    DocuScan &sd = *(DocuScan *) nativeObject;
    sd.setGutterSize(n);
}


JNIEXPORT void
JNICALL
Java_com_locii_docuscanlib_DocuScan_setGuide(JNIEnv *, jobject, jlong nativeObject, jfloat tl_x,
                                             jfloat tl_y, jfloat br_x, jfloat br_y) {

    DocuScan &sd = *(DocuScan *) nativeObject;
    Point_<float> tl = Point2f(tl_x, tl_y);
    Point_<float> br = Point2f(br_x, br_y);
    sd.setGuide(tl, br);
}


}

inline std::ostream &operator<<(std::ostream &s, const DocuScan &p) {
    return s << "[Dist: " << p.getDistance()
             << ", Sharp : " << p.getSharpness()
             << ", Edges : " << p.getEdges()
             << ", Gap : " << p.getMaxLineGap()
             << ", Thrsh: " << p.getLinesThreshold()
             << ", Min Line: " << p.getLineMinWidth()
             << ", Gutter: " << p.getGutterSize()
             << ", TL: " << p.getTopLeft()
             << ", BR: " << p.getBottomRight()
             << ", Dev: " << p.getDevMode() << "]";
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

static Mat *processImage(Mat &srcImage, Mat &mat, Mat &contouredImage1, Mat &contouredImage2,
                         DocuScan &scanParams) {
    Mat gray, blurImage, edged;
    ostringstream msg;


    // create a BW image:
    cvtColor(srcImage, gray, COLOR_BGR2GRAY);


    Mat lap;
    Mat mean(1, 4, CV_64F), stdDev(1, 4, CV_64F);

//    compute sharpness as a standard deviation of the Laplacian
    Laplacian(gray, lap, CV_64F);
    meanStdDev(lap, mean, stdDev);

//    logMeanAndStd(mean, stdDev);

    // require decent degree of sharpness
    if (stdDev.at<double>(0, 0) < scanParams.getSharpness()) {
        msg <<
            "(Failed sharpness test: " << stdDev.at<double>(0, 0) << " < "
            << scanParams.getSharpness() << ")";
        logOStream(msg);
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
    Canny(binary, edged, 1, 3, 3);
    binary.release();
    return extractWithHoughLines(srcImage, mat, contouredImage1, contouredImage2, scanParams,
                                 edged);
}

static int euclideanDistance(const Point &p1, const Point &p2) {
    return static_cast<int>(sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2)));

}

#define TOP 0
#define LEFT 1
#define RIGHT 2
#define BOTTOM 3

class RoiMetaData {

public:
    RoiMetaData(int x, int y, int w, int h) : roi(x, y, w, h) {

    }

    RoiMetaData() : RoiMetaData(0, 0, 0, 0) {
    }

    Rect roi;
    Vec4i selectedLine;
    int distance = INT_MAX;

};

// https://docs.opencv.org/3.4/d9/db0/tutorial_hough_lines.html
static Mat *extractWithHoughLines(const Mat &srcImage, Mat &mat, const Mat &devMat1,
                                  const Mat &devMat2, const DocuScan &scanParams,
                                  Mat &edged) {
    const Point2f &topLeft = scanParams.getTopLeft();
    const Point2f &bottomRight = scanParams.getBottomRight();

    Point2i tl = Point2i(static_cast<int>(topLeft.x), static_cast<int>(topLeft.y));
    Point2i bl = Point2i(static_cast<int>(topLeft.x), static_cast<int>(bottomRight.y));
    Point2i tr = Point2i(static_cast<int>(bottomRight.x), static_cast<int>(topLeft.y));
    Point2i br = Point2i(static_cast<int>(bottomRight.x), static_cast<int>(bottomRight.y));

    if(scanParams.getDevMode()){
        // draw the guide
        rectangle(devMat1,topLeft,bottomRight, Scalar(255,255,128,128),3,LINE_AA);
    }


    int width = tr.x - tl.x;
    int height = bl.y - tl.y;
    int gutter = scanParams.getGutterSize();
    int dw =  gutter;
    int dh = gutter;

    // crate 4 ROI to scan the lines in:
    RoiMetaData roiData[4];
    roiData[TOP] = RoiMetaData(tl.x - dw, tl.y - dh, width + 2 * dw, 2 * dh);
    roiData[BOTTOM] = RoiMetaData(bl.x - dw, bl.y - dh, width + 2 * dw, 2 * dh);
    roiData[LEFT] = RoiMetaData(tl.x - dw, tl.y - dh, 2 * dw, height + 2 * dh);
    roiData[RIGHT] = RoiMetaData(tr.x - dw, tr.y - dh, 2 * dw, height + 2 * dh);


    int distance = scanParams.getDistance();

    // draw the roi on the dev mat
    for (int i = 0; i < 4; i++) {
        rectangle(devMat1, roiData[i].roi, Scalar(128, 0, 255, 255), 4, LINE_AA);
    }

    int success = 0;


    ostringstream msg;
    msg << "Scanning with Params: " << scanParams;
    logOStream(msg);

    // create 4 images from those ROI and extract Hough lines
    for (int j = 0; j < 4; j++) {
        RoiMetaData metaData = roiData[j];
        Mat roiImage = edged(metaData.roi);
        msg << "ROI# " << j << " (" << metaData.roi << ")";
        logOStream(msg);

        vector<Vec4i> linesP; // will hold the results of the detection
        // run the actual detection
        HoughLinesP(roiImage, linesP, 1, CV_PI / 180,
                    scanParams.getLinesThreshold(),
                    scanParams.getLineMinWidth()
                    * MAX(metaData.roi.width, metaData.roi.height) / 100.0,
                    static_cast<double>(scanParams.getMaxLineGap()));


//        for each of the lines detected in this region - inspect the length?
        for (size_t i = 0; i < linesP.size(); i++) {
            Vec4i l = linesP[i];
            // the point of the line
            const Point2i &lp1 = Point(l[0], l[1]);
            const Point2i &lp2 = Point(l[2], l[3]);
            msg << "Inspecting line #" << i << ": {" << lp1 << ", " << lp2 << "}";
            logOStream(msg);

            if (scanParams.getDevMode()) {

//                draw the line
                Point2i p = Point(metaData.roi.x + l[0], metaData.roi.y + l[1]);
                Point2i q = Point(metaData.roi.x + l[2], metaData.roi.y + l[3]);

                line(devMat1, p, q, Scalar(255, 64, 64, 255), 5, LINE_AA);
            }

            // figure out the lines closest to the roi's top and bottom
            //  left + right
            int d1 = euclideanDistance(lp1, metaData.roi.tl()) +
                    euclideanDistance(lp2, metaData.roi.br());
            int d2 = euclideanDistance(lp2, metaData.roi.tl()) +
                    euclideanDistance(lp1, metaData.roi.br());
            if (d1 <= metaData.distance || d2 <= metaData.distance) {
                metaData.distance = MIN(d1, d2);
                metaData.selectedLine = l;
                msg << "Updating candidate for ROI: " << j << ", " << metaData.roi
                    << ", line: " << l
                    << ", distance: " << metaData.distance;
                logOStream(msg);


            }

        }

        // if the distance is too big - terminate
        if (linesP.empty() || metaData.distance > distance) {
            continue;
        }

        success++;
        if (scanParams.getDevMode()) {

            // draw the selected line
            Vec4i l = metaData.selectedLine;
            Point2i pt1 = Point(metaData.roi.x + l[0], metaData.roi.y + l[1]);
            Point2i pt2 = Point(metaData.roi.x + l[2], metaData.roi.y + l[3]);

            line(devMat1, pt1, pt2, Scalar(255, 255, 0, 255), 5, LINE_AA);
        }
    }

    if (success >= scanParams.getEdges()) {
        return &mat;
    } else {
        return NULL;
    }
}


static Mat *extractWithContours(const Mat &srcImage, Mat &mat, const Mat &contouredImage1,
                                const Mat &contouredImage2, const DocuScan &scanParams,
                                Mat &edged) {
    ostringstream msg;
    // find contours in hte canny image (edged)
    int levels = 3;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    vector<vector<Point> > contours0;
    findContours(edged, contours0, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);


    edged.release();

    contours.resize(contours0.size());
    for (size_t k = 0; k < contours0.size(); k++)
        approxPolyDP(Mat(contours0[k]), contours[k], 3, true);

    // see example in https://docs.opencv.org/master/db/d00/samples_2cpp_2squares_8cpp-example.html#a17
// for extracting squares

    // or do this: find the contour with largest area - assume it is the document

    int _levels = levels - 3;
    int maxAreaContour = 0, maxArea = 0;
    vector<Point> approxPoly;
    for (int i = 0; i < contours.size(); i++) {
        int current = contourArea(contours[i], false);

        if (current > maxArea) {
            // check the shape of it:
            vector<Point> approx;
            approxPolyDP(contours[i], approx, arcLength(contours[i], true) * 0.02, true);

            // square contours should have 4 vertices after approximation
            // relatively large area (to filter out noisy contours)
            // and be convex.
            // Note: absolute value of an area is used because
            if (approx.size() == 4 && isContourConvex(approx)) {
                maxArea = current;
                maxAreaContour = i;
                approxPoly = approx; // keep record of hte approximated polygon
            }
        }
    }

    // was a guide provided in the params?
    Point2f topLeft = scanParams.getTopLeft();
    Point2f bottomRight = scanParams.getBottomRight();
    bool hasValidGuide =
            topLeft.x > 0.0 && topLeft.y > 0.0 && bottomRight.x > 0.0 && bottomRight.y > 0.0;

    float guideArea = Rect2f(topLeft, bottomRight).area();

    float area = static_cast<float>(maxArea);

    int imageSize = srcImage.cols * srcImage.rows;
    bool areasMatch = hasValidGuide && abs(guideArea - area) < 0.2f * area;
    bool areaFill = !hasValidGuide && area > -0.6 * 0.6 * imageSize &&
                    area < -0.8 * 0.8 * imageSize;
    bool polyIsSquare = approxPoly.size() == 4;
    msg <<
        "Shape/ Area test: area: " << area
        << ", polyIsSquare: " << polyIsSquare
        << ", areasMatch: " << areasMatch
        << ", areaFill: " << areaFill;

    logOStream(msg);

    if (!polyIsSquare ||
        (!areasMatch && !areaFill)) {

        msg << "Failed shape/ area test:";
        logOStream(msg);


        msg << "\tApprox Poly: Size " << approxPoly.size() << ", area: " << maxArea
            << ", Image size:"
            << imageSize;
        logOStream(msg);

        msg << "\tGuiding rect (valid: " << hasValidGuide << ") :" << topLeft << ","
            << bottomRight
            << " Area: " << guideArea;
        logOStream(msg);

        return NULL;
    }


    drawContours(contouredImage1, contours, maxAreaContour, Scalar(128, 255, 255),
                 3, LINE_AA, hierarchy, abs(_levels));




    // find the rotated rect of the contour (rotated = tilted/skwed)
    RotatedRect rect = minAreaRect(contours[maxAreaContour]);
    Rect boundRect;
    if (hasValidGuide) {

        boundRect = Rect(static_cast<int>(topLeft.x),
                         static_cast<int>(topLeft.y),
                         static_cast<int>(bottomRight.x - topLeft.x),
                         static_cast<int>(bottomRight.y - topLeft.y));
    } else {
        boundRect = boundingRect(contours[maxAreaContour]);
    }

    Point2f box[4];
//    rect.points(box); // the points of the rotated rect

    box[0] = Point2f(boundRect.x, boundRect.y);
    box[1] = Point2f(boundRect.x + boundRect.width, boundRect.y);
    box[2] = Point2f(boundRect.x + boundRect.width, boundRect.y + boundRect.height);
    box[3] = Point2f(boundRect.x, boundRect.y + boundRect.height);

    Scalar blue = Scalar(255, 128, 0, 255);
    Scalar red = Scalar(0, 128, 255, 255);
    Scalar black = Scalar(0, 0, 0, 255);
    Scalar orange = Scalar(0, 255, 220, 255);

    String numbers[] = {"1", "2", "3", "4"};

    double textScale = 5;
    int textThinkness = 4;


    for (int j = 0; j < 4; j++) {
        line(contouredImage1, approxPoly[j], approxPoly[(j + 1) % 4], blue, 3, LINE_AA);
        putText(contouredImage1, numbers[j], approxPoly[j], FONT_HERSHEY_SIMPLEX, textScale,
                blue,
                textThinkness);
        line(contouredImage1, box[j], box[(j + 1) % 4], red, 3, LINE_AA);
        putText(contouredImage1, numbers[j], box[j], FONT_HERSHEY_SIMPLEX, textScale, red,
                textThinkness);
    }
    // draw the bounding rect
    rectangle(contouredImage1, Point(boundRect.x, boundRect.y),
              Point(boundRect.x + boundRect.width, boundRect.y + boundRect.height), black, 3,
              LINE_AA);


    // transform the skewed image
    vector<Point2f> polyPoints;
    vector<Point2f> boxPoints;
    vector<Point2f> boundingRectPoints;
    // poly to box:

    for (int i = 0; i < 4; i++) {
        polyPoints.push_back(Point2f(approxPoly[i].x, approxPoly[i].y));
        boundingRectPoints.push_back(closestPoint(approxPoly[i], box, 4));
    }

    // calcuate the distance from the bounding rectangle to the polygon - smaller distance <-> less skwed image
    double distance = 0;
    for (int j = 0; j < 4; j++) {
        putText(contouredImage1, numbers[j], boundingRectPoints[j], FONT_HERSHEY_SIMPLEX,
                textScale,
                black, textThinkness, LINE_8);
        line(contouredImage1, boundingRectPoints[j], polyPoints[j], orange, 3, LINE_AA);

        distance += pow(boundingRectPoints[j].x - polyPoints[j].x, 2) +
                    pow(boundingRectPoints[j].y - polyPoints[j].y, 2);
    }

    msg << "Distance: " << distance;
    logOStream(msg);

    if (distance > (double) scanParams.getDistance()) {
        msg <<
            "(Failed distance test: " << distance << " > "
            << scanParams.getDistance() << ")";
        logOStream(msg);
        return NULL;
    }

    Mat transmtx = getPerspectiveTransform(polyPoints, boundingRectPoints);
    Mat transformed = Mat::zeros(srcImage.rows, srcImage.cols, CV_8UC3);
    warpPerspective(srcImage, transformed, transmtx, srcImage.size());

    // box to rect:

    polyPoints.clear();
    boundingRectPoints.clear();

    if (hasValidGuide) {
        boundingRectPoints.push_back(Point2f(topLeft));
        boundingRectPoints.push_back(Point2f(bottomRight.x, topLeft.y));
        boundingRectPoints.push_back(Point2f(bottomRight));
        boundingRectPoints.push_back(Point2f(topLeft.x, bottomRight.y));

    } else {
        float h = rect.size.height;//MIN(rect.size.height,rect.size.width);
        float w = rect.size.width;//MAX(rect.size.height,rect.size.width);
        boundingRectPoints.push_back(Point2f(boundRect.x, boundRect.y));
        boundingRectPoints.push_back(Point2f(boundRect.x, boundRect.y + h));
        boundingRectPoints.push_back(Point2f(boundRect.x + w, boundRect.y));
        boundingRectPoints.push_back(Point2f(boundRect.x + w, boundRect.y + h));
    }

    for (int i = 0; i < 4; ++i) {
        Point p = Point(boundingRectPoints[i].x, boundingRectPoints[i].y);
        polyPoints.push_back(closestPoint(p, box, 4));

    }


    for (int j = 0; j < 4; j++) {
        line(contouredImage2, polyPoints[j], polyPoints[(j + 1) % 4], red, 3, LINE_AA);
        putText(contouredImage2, numbers[j], polyPoints[j], FONT_HERSHEY_SIMPLEX, textScale,
                red,
                textThinkness);
        line(contouredImage2, boundingRectPoints[j], boundingRectPoints[(j + 1) % 4], orange, 3,
             LINE_AA);
        putText(contouredImage2, numbers[j], boundingRectPoints[j], FONT_HERSHEY_SIMPLEX,
                textScale,
                orange, textThinkness);
        line(contouredImage2, boundingRectPoints[j], polyPoints[j], blue, 3, LINE_AA);
    }


    // transform and crop
    transmtx = getPerspectiveTransform(polyPoints, boundingRectPoints);
    warpPerspective(transformed, transformed, transmtx, transformed.size());

    Mat cropped(transformed, boundRect);
    cropped.copyTo(mat);
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
