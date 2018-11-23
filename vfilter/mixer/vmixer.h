#pragma once

#include <string>
#include <map>
#include <vector>

#include "opencv2/opencv.hpp"

#include "vfilter/mixer/cvx_text.h"
#include "algo/stub/object_type.h"
#include "vfilter/core/object_sink.h"

using namespace std;

namespace vf {
static const char *DEFAULT_FONT = "/usr/share/fonts/truetype/simsun.ttf";
static const int FONT_MIN_SIZE = 16;	//字体最小号
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
        objSink.GetShowableObjects(tmpObjs1, tmpObjs2);
        doMixFrame(frame, tmpObjs1, tmpObjs2);
    }

protected:
    virtual void doMixFrame(cv::Mat &frame, vector<T> &objs1, vector<T> &objs2) {}

    //画目标的矩形框
    virtual void mixObjectRectangle(cv::Mat &frame, int32_t x, int32_t y, int32_t w, int32_t h,
                                    CvScalar color = CV_RGB(255, 255, 255)) {
        //check param
        if (x < 0 || y < 0 || w <= 0 || h <= 0) {
            LOG_WARN("Invalid param x={}, y={} w={} h={}", x, y, w, h);
            return;
        }

        //计算矩形的4个转角坐标, a,b,c,d 表示矩形的4个顶点（顺时针）
        int32_t len = 8;
        cv::Point a(x, y), b(x + w, y), c(x + w, y + h), d(x, y + h);
        cv::Point a1(a.x, a.y+ len), a2(a.x+ len, a.y), b1(b.x, b.y+len), b2(b.x-len, b.y), c1(c.x, c.y-len), c2(c.x-len, c.y),
        d1(d.x, d.y-len), d2(d.x+len, d.y);

        //画矩形的4个转角
        int32_t thickness = 3;
        cv::line(frame, a, a1, color, thickness);
        cv::line(frame, a, a2, color, thickness);
        cv::line(frame, b, b1, color, thickness);
        cv::line(frame, b, b2, color, thickness);
        cv::line(frame, c, c1, color, thickness);
        cv::line(frame, c, c2, color, thickness);
        cv::line(frame, d, d1, color, thickness);
        cv::line(frame, d, d2, color, thickness);

        //画矩形边框
        thickness = 1;
        cv::rectangle(frame, cvPoint(x, y), cvPoint(x + w, y + h), color, thickness);
    }

    //画目标的属性文字
    virtual void mixObjectAttributeText(cv::Mat &frame, int32_t x, int32_t y, int32_t w, int32_t h,
                                        vector<algo::Attribute> &attrs, CvScalar color = CV_RGB(255, 255, 255)) {
        //check param
        if (x < 0 || y < 0 || w <= 0 || h <= 0) {
            LOG_WARN("Invalid param x={}, y={} w={} h={}", x, y, w, h);
            return;
        }

        // 根据目标的远近，计算字体大小
        int32_t fontSize = FONT_MIN_SIZE + (w / (frame.cols / (FONT_MAX_SIZE - FONT_MIN_SIZE)));
        cv::Scalar size{ (double)fontSize, 0.5, 0.1, 0 };
        text_.setFont(nullptr, &size, nullptr, nullptr);

        // 计算字符中的宽字符数目和ascii字符数目
        uint32_t maxWcharFontNum = 0;
        uint32_t maxAsciiFontNum = 0;
        for (auto &a : attrs) {
            if (a.name.empty()) {
                continue;
            }

            wchar_t text[200] = { 0 };
            swprintf(text, 200, L"%hs", a.name.c_str());

            uint32_t lineWcharFontNum = 0;
            uint32_t lineAsciiFontNum = 0;
            for (int i = 0; text[i] != '\0'; ++i) {
                if (!isascii(text[i])) {
                    lineWcharFontNum++;
                } else {
                    lineAsciiFontNum++;
                }
            }
            maxWcharFontNum = (lineWcharFontNum > maxWcharFontNum) ? lineWcharFontNum : maxWcharFontNum;
            maxAsciiFontNum = (lineAsciiFontNum > maxAsciiFontNum) ? lineAsciiFontNum : maxAsciiFontNum;
        }

        uint32_t maxTextPixNum = (maxWcharFontNum * fontSize) + (maxAsciiFontNum * fontSize / 2);

        const int32_t MARGIN_LEFT = 6;	//字体与左边界的间隔
        const int32_t MARGIN_TOP = 4;	//字体与上边界的间隔

        int idx = 0;
        for (auto &a : attrs) {
            if (a.name.empty()) {
                continue;
            }

            //start x, start y
            int32_t sx = x, sy = y;
            //如果字体的宽度大于目标的宽度，把字体写在目标框右边
            if (textRightForce() || maxTextPixNum > (uint32_t)w) {
                sx = x + w;
            }

            //字体的开始坐标x，y
            int x1 = sx + MARGIN_LEFT;
            int y1 = sy + idx * (fontSize + MARGIN_TOP);
            int w1 = maxTextPixNum;
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

            // 如果宽或者高计算出来为0，说明到最边上了，不显示
            if (w1 == 0 || h1 == 0) {
                continue;
            }
            // y方向超过最大，不显示多余的
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
            text_.putText(frame, msg, cvPoint(x1, y1 + fontSize), color);

            idx++;
        }
    }

    //字体是否强制显示在右边
    virtual bool textRightForce() {
        return true;
    }

private:
    CvxText text_;
    string type_;
};


}