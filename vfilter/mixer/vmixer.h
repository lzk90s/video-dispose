#pragma once

#include <string>
#include <map>
#include <mutex>
#include <vector>

#include "opencv/cv.h"
#include "opencv2/opencv.hpp"

#include "vfilter/setting.h"
#include "vfilter/mixer/cvx_text.h"
#include "vfilter/mixer/cvx_text.h"
#include "algo/stub/object_type.h"
#include "vfilter/object_sink.h"

using namespace std;

namespace vf {
static const char *DEFAULT_FONT = "/usr/share/fonts/truetype/simsun.ttf";
static const int FONT_MIN_SIZE = 20;	//字体最小号
static const int FONT_MAX_SIZE = 40;	//字体最大号


template<class T>
class VMixer {
public:
    VMixer(const string &type)
        : text_(DEFAULT_FONT),
          type_(type) {
        cv::Scalar size{ FONT_MIN_SIZE, 0.5, 0.1, 0 };
        text_.setFont(nullptr, &size, nullptr, nullptr);
    }

    virtual void MixFrame(cv::Mat &frame, ObjectSink<T> &objSink) {
        vector<T> tmpObjs1, tmpObjs2;
        objSink.GetLatestObjects(tmpObjs1, tmpObjs2);
        doMixFrame(frame, tmpObjs1, tmpObjs2);
    }

protected:
    virtual void doMixFrame(cv::Mat &frame, vector<T> &objs1, vector<T> &objs2) {}

    //画目标的矩形框
    virtual void mixObjectRectangle(cv::Mat &frame, int32_t x, int32_t y, int32_t w, int32_t h,
                                    CvScalar color= CV_RGB(255,255,255)) {
        int32_t thickness = 3;
        cv::rectangle(frame, cvPoint(x, y), cvPoint(x + w, y + h), color, thickness, 1, 0);
    }

    //画目标的属性文字
    virtual void mixObjectAttributeText(cv::Mat &frame, int32_t x, int32_t y, int32_t w, int32_t h,
                                        vector<algo::Attribute> &attrs, CvScalar color= CV_RGB(255,255,255)) {
        // 根据目标的远近，计算字体大小
        int32_t fontSize = FONT_MIN_SIZE + (w / (frame.cols / (FONT_MAX_SIZE - FONT_MIN_SIZE)));
        cv::Scalar size{ (double)fontSize, 0.5, 0.1, 0 };
        text_.setFont(nullptr, &size, nullptr, nullptr);

        // 计算所有属性最大字符个数
        uint32_t maxFontNum = 0;
        for (auto &a : attrs) {
            if (a.name.empty()) {
                continue;
            }
            wchar_t msg[200] = { 0 };
            swprintf(msg, 200, L"%hs", a.name.c_str());
            if (wcslen(msg) > maxFontNum) {
                maxFontNum = wcslen(msg);
            }
        }

        const int32_t MARGIN_LEFT = 8;	//字体与左边界的间隔
        const int32_t MARGIN_TOP = 6;	//字体与上边界的间隔

        int idx = 0;
        for (auto &a : attrs) {
            if (a.name.empty()) {
                continue;
            }

            //start x, start y
            int32_t sx = x + w, sy = y;
            //如果框的右边离图像的右边距离太近，避免字被截断，则字写在框的左边
            if ((uint32_t)(frame.cols - sx) < (maxFontNum*fontSize)) {
                sx = x;
            }

            //字体的开始坐标x，y
            int x1 = sx + MARGIN_LEFT;
            int y1 = sy + idx * (fontSize + MARGIN_TOP);
            int w1 = maxFontNum * fontSize;
            int h1 = fontSize;

            //截断处理
            if (x1 >= frame.cols) {
                x1 = frame.cols;
            }
            if (y1 >= frame.rows) {
                y1 = frame.rows;
            }
            if (x1 + w1 >= frame.cols) {
                w1 = frame.cols - x1;
            }
            if (y1 + h1 >= frame.rows) {
                h1 = frame.rows - y1;
            }

            // 超过最大，不显示
            if (y1 + fontSize > frame.rows) {
                LOG_DEBUG("Ignore attribute {} to mix", a.name.c_str());
                continue;
            }

            //画字体阴影背景(x1,y1,字体最大宽度,字体高度)
            cv::Rect rect = cv::Rect(x1, y1, w1, h1);
            cv::Mat roi = frame(rect);
            cv::Mat mask = roi.clone();
            cv::rectangle(mask, { 0,0, mask.cols, mask.rows }, CV_RGB(50, 50, 50), -1);
            cv::addWeighted(roi, 0.3, mask, 0.7, 0, roi);

            //叠加文字
            wchar_t msg[200] = { 0 };
            swprintf(msg, 200, L"%hs", a.name.c_str());
            //字体是以字的下面作为坐标的，所以，y1是字的上部坐标，所以坐标再往下移动一个字的高度
            text_.putText(frame, msg, cvPoint(x1, y1+ fontSize), color);

            idx++;
        }
    }

private:
    CvxText text_;
    string type_;
};


}