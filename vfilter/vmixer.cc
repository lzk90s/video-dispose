#include <iostream>

#include "vfilter/vmixer.h"

#include "opencv/cv.h"
#include "opencv2/opencv.hpp"
#include "vfilter/cvx_text.h"

void VMixer::MixFrame(cv::Mat &frame) {
    static int i = 0, j=0;
    i++;
    j += 2;
    if (i >= 600) {
        i = 0;
        j = 0;
    }

    Cv310Text text("/usr/share/fonts/truetype/simsun.ttf");
    const wchar_t msg[] = L"我是矩形框";
    float p = 1;
    text.setFont(NULL, NULL, NULL, &p);

    text.putText(frame, msg, cvPoint(j, i+100), CV_RGB(255, 0, 0));
    cv::rectangle(frame, cvPoint(j, i+100), cvPoint(j+300, i+300), cv::Scalar(0, 255, 0), 5, 1, 0);
}
