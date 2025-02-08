#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

using namespace cv;
using namespace std;

void onTrackbar(int, void*) {}

int main() {
    string filename = "C:/Users/linglitel/Desktop/opencv_homework.mp4";
    VideoCapture cap(filename);

    if (!cap.isOpened()) {
        return -1;
    }

    Mat frame, mid, dilated, edge,debug;
    vector<Mat> channels;
    int kernel_size = 2;
    Mat kernel = getStructuringElement(MORPH_RECT, Size(kernel_size, kernel_size));
    vector<vector<Point> > contours;
    while (true) {
        cap >> frame;
        if (frame.empty()) {
            break;
        }
        imshow("Original", frame);
        split(frame, channels);
        dilate(channels[0], dilated, kernel);
        threshold(dilated, mid, 150, 255, THRESH_BINARY);
        edge = mid;
        //Canny(mid, edge, 150, 200, 3, true);
        findContours(edge.clone(), contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
        Mat result = frame.clone();
        for (const auto& contour : contours) {
            auto rotatedRect = minAreaRect(contour);
            Point2f vertices[4];
            rotatedRect.points(vertices);
            int w = rotatedRect.size.width;
            int l = rotatedRect.size.height;
            if (l==0||w==0)
            {
                continue;
            }
            if (w > l) l = l ^ w, w = l ^ w, l = l ^ w;
            if (l>20)
            {
                continue;
            }
            //if (double(l) / double(w) < 1) {
            //    continue;
            //}
        
            //if (double(l) / double(w) <= 2) {
            //continue;
            //}

            for (int j = 0; j < 4; j++) {
                line(result, vertices[j], vertices[(j + 1) % 4], Scalar(0, 0, 255), 1);
            }
            debug = result.clone();
            Point2f center = rotatedRect.center;
            string text = "W:" + to_string(w) + " L:" + to_string(l);
            putText(debug, text, center, FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 1);
            imshow("Rotated Rectangles", result);
            imshow("Debug Window", debug);
        }
        waitKey(30);
    }

    return 0;
}
