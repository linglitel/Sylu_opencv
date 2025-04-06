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

    Mat frame, mid, dilated, edge, debug;
    vector<Mat> channels;
    int kernel_size = 2;
    Mat kernel = getStructuringElement(MORPH_RECT, Size(kernel_size, kernel_size));

    while (true) {
        cap >> frame;
        if (frame.empty()) {
            break;
        }
        imshow("Original", frame);
        split(frame, channels); // 假设使用蓝色通道
        dilate(channels[0], dilated, kernel);
        threshold(dilated, mid, 0, 255, THRESH_OTSU); // 使用Otsu阈值
        edge = mid;

        vector<vector<Point>> contours;
        findContours(edge.clone(), contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        vector<RotatedRect> validRects;
        for (const auto& contour : contours) {
            if (contour.size() < 5) continue; // 轮廓点太少跳过

            RotatedRect rRect = minAreaRect(contour);
            float w = rRect.size.width;
            float l = rRect.size.height;

            // 确保w是短边，l是长边
            if (w > l) swap(w, l);
            double aspectRatio = l / w;

            // 宽高比条件：2.5到4.5之间
            if (aspectRatio < 2.5 || aspectRatio > 4.5) continue;

            // 角度条件：接近水平或垂直
            float angle = rRect.angle;
            if (abs(angle) > 15.0 && abs(angle + 90.0) > 15.0) continue;

            validRects.push_back(rRect);
        }

        vector<pair<RotatedRect, RotatedRect>> armorPairs;
        for (size_t i = 0; i < validRects.size(); ++i) {
            for (size_t j = i+1; j < validRects.size(); ++j) {
                const RotatedRect& r1 = validRects[i];
                const RotatedRect& r2 = validRects[j];

                // 垂直距离不超过长边的30%
                float dy = abs(r1.center.y - r2.center.y);
                float avgLength = (r1.size.height + r2.size.height) * 0.5;
                if (dy > avgLength * 0.3) continue;

                // 尺寸相似度
                float w1 = r1.size.width, w2 = r2.size.width;
                float h1 = r1.size.height, h2 = r2.size.height;
                if (abs(w1 - w2)/(w1 + w2) > 0.25) continue;
                if (abs(h1 - h2)/(h1 + h2) > 0.25) continue;

                // 角度相近
                float angleDiff = abs(r1.angle - r2.angle);
                if (angleDiff > 20.0 && angleDiff < 160.0) continue;

                armorPairs.emplace_back(r1, r2);
            }
        }

        Mat result = frame.clone();
        debug = frame.clone();

        for (const auto& pair : armorPairs) {
            const RotatedRect& r1 = pair.first;
            const RotatedRect& r2 = pair.second;

            // 绘制装甲板轮廓
            Point2f vertices1[4], vertices2[4];
            r1.points(vertices1);
            r2.points(vertices2);

            for (int j = 0; j <4; j++) {
                line(result, vertices1[j], vertices1[(j+1)%4], Scalar(0,255,0),2);
                line(result, vertices2[j], vertices2[(j+1)%4], Scalar(0,255,0),2);
            }
            Point2f center = (r1.center + r2.center)/2;
            putText(result, "Armor", center, FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0,255,0),2);
        }

        imshow("Rotated Rectangles", result);
        imshow("Debug Window", debug);
        waitKey(30);
    }

    return 0;
}
