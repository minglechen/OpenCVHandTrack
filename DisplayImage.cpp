#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include <iostream>
#include <vector>
#include <opencv2/features2d.hpp>
#include "UDPSend.h"
using namespace cv;
using std::vector;
const int max_value_H = 360 / 2;
const int max_value = 255;
const String window_capture_name = "Video Capture";
const String window_detection_name = "Object Detection";
int low_H = 0, low_S = 0, low_V = 110;
int high_H = max_value_H, high_S = max_value, high_V = max_value;
int top, bottom, left, right;
int borderType = BORDER_CONSTANT;
int size_thresh = 100;
int thresh = 100;
Mat frame_threshold, frame;
RNG rng(12342);


static void on_low_H_thresh_trackbar(int, void *) {
    low_H = min(high_H - 1, low_H);
    setTrackbarPos("Low H", window_detection_name, low_H);
}

static void on_high_H_thresh_trackbar(int, void *) {
    high_H = max(high_H, low_H + 1);
    setTrackbarPos("High H", window_detection_name, high_H);
}

static void on_low_S_thresh_trackbar(int, void *) {
    low_S = min(high_S - 1, low_S);
    setTrackbarPos("Low S", window_detection_name, low_S);
}

static void on_high_S_thresh_trackbar(int, void *) {
    high_S = max(high_S, low_S + 1);
    setTrackbarPos("High S", window_detection_name, high_S);
}

static void on_low_V_thresh_trackbar(int, void *) {
    low_V = min(high_V - 1, low_V);
    setTrackbarPos("Low V", window_detection_name, low_V);
}

static void on_high_V_thresh_trackbar(int, void *) {
    setTrackbarPos("Size Thresh", window_detection_name, size_thresh);
}

static void on_size_thresh_trackbar(int, void *) {
    high_V = max(high_V, low_V + 1);
    setTrackbarPos("High V", window_detection_name, high_V);
}

void thresh_callback(int, void *);

int main(int argc, char *argv[]) {
    VideoCapture cap(argc > 1 ? atoi(argv[1]) : 0);
    namedWindow(window_capture_name);
    namedWindow(window_detection_name);
    // Trackbars to set thresholds for HSV values
    createTrackbar("Low H", window_detection_name, &low_H, max_value_H, on_low_H_thresh_trackbar);
    createTrackbar("High H", window_detection_name, &high_H, max_value_H, on_high_H_thresh_trackbar);
    createTrackbar("Low S", window_detection_name, &low_S, max_value, on_low_S_thresh_trackbar);
    createTrackbar("High S", window_detection_name, &high_S, max_value, on_high_S_thresh_trackbar);
    createTrackbar("Low V", window_detection_name, &low_V, max_value, on_low_V_thresh_trackbar);
    createTrackbar("High V", window_detection_name, &high_V, max_value, on_high_V_thresh_trackbar);
    createTrackbar("Size Thresh", window_detection_name, &size_thresh, 1000000, on_size_thresh_trackbar);
    Mat frame_HSV;
    while (true) {
        cap >> frame;
        if (frame.empty()) {
            break;
        }
        // Convert from BGR to HSV colorspace
        top = (int) (0.05 * frame.rows);
        bottom = top;
        left = (int) (0.05 * frame.cols);
        right = left;
        cvtColor(frame, frame_HSV, COLOR_BGR2HSV);
        // Detect the object based on HSV Range Values
        inRange(frame_HSV, Scalar(low_H, low_S, low_V), Scalar(high_H, high_S, high_V), frame_threshold);
        copyMakeBorder(frame_threshold, frame_threshold, top, bottom, left, right, borderType, Scalar(0, 0, 0));
        // Show the frames



        Mat kernel = cv::getStructuringElement(MORPH_ELLIPSE, Size(9, 9));
        erode(frame_threshold, frame_threshold, kernel);
        erode(frame_threshold, frame_threshold, kernel);
        erode(frame_threshold, frame_threshold, kernel);

        morphologyEx(frame_threshold, frame_threshold, MORPH_DILATE, kernel);
        morphologyEx(frame_threshold, frame_threshold, MORPH_DILATE, kernel);
        imshow(window_detection_name, frame_threshold);

        imshow(window_capture_name, frame);
        //imshow(window_detection_name, frame_threshold);
        char key = (char) waitKey(30);
        if (key == 'q' || key == 27) {
            break;
        }
        //blur(frame_threshold, frame_threshold, Size(5,5));

        const int max_thresh = 255;
        createTrackbar("Canny thresh:", window_detection_name, &thresh, 20, thresh_callback);
        //frame_threshold.convertTo(frame_threshold, CV_32SC1);
        thresh_callback(0, 0);


    }
    return 0;
}


void thresh_callback(int, void *) {
    Mat canny_output;
    Canny(frame_threshold, canny_output, thresh, thresh * 2);
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    //frame_threshold.convertTo(frame_threshold, CV_32SC1);
    findContours(frame_threshold, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    if (contours.size() == 0) return;
    vector<vector<Point> > contours_poly(contours.size());
    vector<Rect> boundRect(contours.size());
    vector<Point2f> centers(contours.size());

    vector<float> radius(contours.size());
    double largestArea = 0.0;
    int largestIndex = 0;
    for (size_t i = 0; i < contours.size(); ++i) {
        if (contourArea(contours[i]) > largestArea) {
            largestArea = contourArea(contours[i]);
            largestIndex = i;
        }
    }
    // and now we got contours[largestIndex]
    // this is (probably) the contour of our hand

    // below are some code to draw a min rect and a ellipse
    // we don't need them right now

    /*************************************
//    RotatedRect minRect, minEllipse;
//    minRect = minAreaRect(contours[largestIndex]);
//    if(contours[largestIndex].size() > 5 )
//    {
//        minEllipse = fitEllipse( contours[largestIndex] );
//    }
//
//    approxPolyDP(contours[largestIndex], contours_poly[largestIndex], 3, true);
//    boundRect[largestIndex] = boundingRect(contours_poly[largestIndex]);
//    minEnclosingCircle(contours_poly[largestIndex], centers[largestIndex], radius[largestIndex]);
//
//    Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
//    Scalar color = Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));
//    drawContours(drawing, contours_poly, (int) largestIndex, color);
//    //rectangle(drawing, boundRect[largestIndex].tl(), boundRect[largestIndex].br(), color, 2);
//    //circle(drawing, centers[largestIndex], (int) radius[largestIndex], color, 2);
//    ellipse( drawing, minEllipse, color, 2 );
//    Point2f rect_points[4];
//    minRect.points( rect_points );
//
//    for ( int j = 0; j < 4; j++ )
//    {
//        line( drawing, rect_points[j], rect_points[(j+1)%4], color );
//    }
//    imshow("Contours", drawing);
     ************************************************/
    vector<vector<Point>> hull = vector<vector<Point>>(1);
    vector<vector<int>> hullsI(1);
    vector<Vec4i> defect = vector<Vec4i>(1);
    convexHull(contours[largestIndex], hull[0], false);
    convexHull(contours[largestIndex], hullsI[0], false);
    convexityDefects(Mat(contours[largestIndex]), hullsI[0],defect);
    Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
    Scalar color = Scalar( 255, 0, 0);

    drawContours( drawing, contours, largestIndex, color );
    Scalar point_color = Scalar(0,255,0);
    float curlicue_height = 999999.0;
    int curlicue_index = 0;
    int second_highest_index = 0;
    for (int i = 0; i < defect.size(); ++i){
        if (contours[largestIndex][defect[i][2]].y < curlicue_height &&
                defect[i][3]/256 > 75){
            curlicue_height = contours[largestIndex][defect[i][2]].y;
            second_highest_index = curlicue_index;
            curlicue_index = i;
        }
    }
    int sec_start_x = contours[largestIndex][defect[second_highest_index][0]].x;
    int sec_start_y = contours[largestIndex][defect[second_highest_index][0]].y;
    int sec_end_x = contours[largestIndex][defect[second_highest_index][1]].x;
    int sec_end_y = contours[largestIndex][defect[second_highest_index][1]].y;
    if (   (sec_start_x > sec_end_x) && (sec_start_y > sec_end_y) && contours[largestIndex][defect[second_highest_index][2]].x > contours[largestIndex][defect[curlicue_index][2]].x){
        curlicue_index = second_highest_index;
    }

    int shoot = 0;
    int start_x = contours[largestIndex][defect[curlicue_index][0]].x;
    int start_y = contours[largestIndex][defect[curlicue_index][0]].y;
    int end_x = contours[largestIndex][defect[curlicue_index][1]].x;
    int end_y = contours[largestIndex][defect[curlicue_index][1]].y;
    int curlicue_x = contours[largestIndex][defect[curlicue_index][2]].x;
    int curlicue_y = contours[largestIndex][defect[curlicue_index][2]].y;
    if ( (start_x > end_x)  && start_y > end_y){
        drawContours(drawing, hull,0, color);
        shoot = 0;
        circle(drawing, contours[largestIndex][defect[curlicue_index][2]], 2, point_color, 5 );
        circle(drawing, contours[largestIndex][defect[curlicue_index][0]], 2, point_color, 5 );
        circle(drawing, contours[largestIndex][defect[curlicue_index][1]], 2, point_color, 5 );
    }
    else {
        color = Scalar(0,0,255);
        shoot = 1;
        drawContours(drawing, hull,0, color);
        circle(drawing, contours[largestIndex][defect[curlicue_index][2]], 2, point_color, 5 );
        circle(drawing, contours[largestIndex][defect[curlicue_index][0]], 2, point_color, 5 );
        circle(drawing, contours[largestIndex][defect[curlicue_index][1]], 2, point_color, 5 );
    }

//    Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
//    vector<vector<Point> >hull( contours.size() );
//    // Int type hull
//    vector<vector<int>> hullsI( contours.size() );
//    // Convexity defects
//    vector<vector<Vec4i>> defects( contours.size() );
//
//    convexHull( Mat(contours[largestIndex]), hull[largestIndex], false );
//    convexHull( Mat(contours[largestIndex]), hullsI[largestIndex], false );
//    convexityDefects(Mat(contours[largestIndex]),hullsI[largestIndex], defects[largestIndex]);
//
//    int i = largestIndex;
//    Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
//    drawContours( drawing, contours, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
//    drawContours( drawing, hull, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
//
//    vector<Vec4i>::iterator d =defects[i].begin();
//
//    while( d!=defects[i].end() ) {
//        Vec4i& v=(*d);
//        //if(IndexOfBiggestContour == i)
//        {
//
//            int startidx=v[0];
//            Point ptStart( contours[i][startidx] ); // point of the contour where the defect begins
//            int endidx=v[1];
//            Point ptEnd( contours[i][endidx] ); // point of the contour where the defect ends
//            int faridx=v[2];
//            Point ptFar( contours[i][faridx] );// the farthest from the convex hull point within the defect
//            int depth = v[3] / 256; // distance between the farthest point and the convex hull
//
//            if(depth > 20 && depth < 80)
//            {
//                line( drawing, ptStart, ptFar, CV_RGB(0,255,0), 2 );
//                line( drawing, ptEnd, ptFar, CV_RGB(0,255,0), 2 );
//                circle( drawing, ptStart,   4, Scalar(255,0,100), 2 );
//                circle( drawing, ptEnd,   4, Scalar(255,0,100), 2 );
//                circle( drawing, ptFar,   4, Scalar(100,0,255), 2 );
//            }
//
//            /*printf("start(%d,%d) end(%d,%d), far(%d,%d)\n",
//                ptStart.x, ptStart.y, ptEnd.x, ptEnd.y, ptFar.x, ptFar.y);*/
//        }
//        d++;
    //}





    imshow( "Hull demo", drawing );
    // send the information
    // start is the index finger tip
    // end is the thumb finger tip
    String tosend = std::to_string(shoot) + " " + std::to_string(start_x) + " "
                    + std::to_string(start_y) + " " +std::to_string(end_x) + " "
                    + std::to_string(end_y) + " " + std::to_string(curlicue_x) + " "
                    + std::to_string(curlicue_y);
    std::cout << tosend << std::endl;
    send(tosend);
}

