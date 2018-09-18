#include <iostream>

#include "vfilter/vmixer.h"

#include "opencv/cv.h"
#include "opencv2/opencv.hpp"
#include "vfilter/cvx_text.h"
#include "common/helper/stringconv.h"

namespace vf {
static const char *DEFAULT_FONT = "/usr/share/fonts/truetype/simsun.ttf";
static const int DEFAULT_FONT_SIZE = 20;

VMixer::VMixer() : text_(DEFAULT_FONT) {
    float p = 1; //透明度
    cv::Scalar size{ 20, 0.5, 0.1, 0 };
    text_.setFont(nullptr, &size, nullptr, &p);
}

void VMixer::MixFrame(cv::Mat &frame, TargetMap &tm) {
    if (tm.empty()) {
        return;
    }

    for (auto t : tm) {
        // 画矩形框
        Rect &rect = t.second->rect;
        cv::rectangle(frame, cvPoint(rect.x, rect.y), cvPoint(rect.x + rect.w, rect.y+rect.h), CV_RGB(255,0,0), 3, 1, 0);

        int idx = 0;
        for (auto a : t.second->attrs) {
            if (!a.visable) {
                continue;
            }
            // 写属性
            int x = rect.x + rect.w + 2;
            int y = rect.y + (idx * DEFAULT_FONT_SIZE) + 2;
            std::wstring msg = StringToWString(a.value);
            text_.putText(frame, msg.c_str(), cvPoint(x, y), CV_RGB(255, 0, 0));
        }
    }
}

}
