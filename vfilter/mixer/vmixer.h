#pragma once

#include <string>
#include <map>
#include <mutex>


#include "opencv/cv.h"
#include "opencv2/opencv.hpp"
#include "vfilter/mixer/cvx_text.h"
#include "vfilter/mixer/cvx_text.h"
#include "algo/stub/object_type.h"

using namespace std;

namespace vf {
static const char *DEFAULT_FONT = "/usr/share/fonts/truetype/simsun.ttf";
static const int DEFAULT_FONT_SIZE = 20;

class VMixer {
public:
    VMixer() : text_(DEFAULT_FONT) {
        cv::Scalar size{ 18, 0.5, 0.1, 0 };
        text_.setFont(nullptr, &size, nullptr, nullptr);
    }

    virtual void MixFrame(cv::Mat &frame) = 0;

protected:
    //画目标的矩形框
    virtual void mixObjectRectangle(cv::Mat &frame, int32_t x, int32_t y, int32_t w, int32_t h,
                                    CvScalar color= CV_RGB(255,255,255)) {
        int32_t thickness = 2;
        cv::rectangle(frame, cvPoint(x, y), cvPoint(x + w, y + h), color, thickness, 1, 0);
    }

    //画目标的属性文字
    virtual void mixObjectAttributeText(cv::Mat &frame, int32_t x, int32_t y, int32_t w, int32_t h,
                                        vector<algo::Attribute> &attrs, CvScalar color= CV_RGB(255,255,255)) {
        // 计算所有属性最大字符个数
        int32_t maxFontNum = 0;
        for (auto a : attrs) {
            wchar_t msg[200] = { 0 };
            swprintf(msg, 200, L"%hs", a.name.c_str());
            if (wcslen(msg) > maxFontNum) {
                maxFontNum = wcslen(msg);
            }
        }

        int idx = 0;
        for (auto a : attrs) {
            //start x, start y
            int32_t sx = x + w, sy = y;
            //如果框的右边离图像的右边距离太近，避免字被截断，则字写在框的左边
            if ((frame.cols - sx) < (maxFontNum*DEFAULT_FONT_SIZE)) {
                sx = x;
            }

            const int32_t MARGIN_LEFT = 8;	//字体与左边界的间隔
            const int32_t MARGIN_TOP = 4;	//字体与上边界的间隔

            //字体的开始坐标x，y
            int x1 = sx + MARGIN_LEFT;
            int y1 = sy + ((idx + 1) * DEFAULT_FONT_SIZE) + MARGIN_TOP;

#if 0
            //画字体阴影背景
            cv::Rect rect = cv::Rect(x1, y1, maxFontNum*DEFAULT_FONT_SIZE, DEFAULT_FONT_SIZE);
            cv::Mat roi = frame(rect);
            cv::Mat mask = roi.clone();
            cv::rectangle(mask, { 0,0, mask.cols, mask.rows }, CV_RGB(50, 50, 50), -1);
            cv::addWeighted(roi, 0.3, mask, 0.7, 0, roi);
#endif
            //叠加文字
            wchar_t msg[200] = { 0 };
            swprintf(msg, 200, L"%hs", a.name.c_str());
            text_.putText(frame, msg, cvPoint(x1, y1), color);

            idx++;
        }
    }

protected:
    CvxText text_;
};


}