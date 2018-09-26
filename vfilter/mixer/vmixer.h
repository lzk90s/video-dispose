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
static const int FONT_MIN_SIZE = 20;	//������С��
static const int FONT_MAX_SIZE = 40;	//��������

class VMixer {
public:
    VMixer() : text_(DEFAULT_FONT) {
        cv::Scalar size{ 18, 0.5, 0.1, 0 };
        text_.setFont(nullptr, &size, nullptr, nullptr);
    }

    virtual void MixFrame(cv::Mat &frame) = 0;

protected:
    //��Ŀ��ľ��ο�
    virtual void mixObjectRectangle(cv::Mat &frame, int32_t x, int32_t y, int32_t w, int32_t h,
                                    CvScalar color= CV_RGB(255,255,255)) {
        int32_t thickness = 2;
        cv::rectangle(frame, cvPoint(x, y), cvPoint(x + w, y + h), color, thickness, 1, 0);
    }

    //��Ŀ�����������
    virtual void mixObjectAttributeText(cv::Mat &frame, int32_t x, int32_t y, int32_t w, int32_t h,
                                        vector<algo::Attribute> &attrs, CvScalar color= CV_RGB(255,255,255)) {
        // ����Ŀ���Զ�������������С
        int32_t fontSize = FONT_MIN_SIZE + (w / (frame.cols / (FONT_MAX_SIZE - FONT_MIN_SIZE)));

        cv::Scalar size{ (double)fontSize, 0.5, 0.1, 0 };
        text_.setFont(nullptr, &size, nullptr, nullptr);

        // ����������������ַ�����
        int32_t maxFontNum = 0;
        for (auto a : attrs) {
            wchar_t msg[200] = { 0 };
            swprintf(msg, 200, L"%hs", a.name.c_str());
            if (wcslen(msg) > maxFontNum) {
                maxFontNum = wcslen(msg);
            }
        }

        const int32_t MARGIN_LEFT = 8;	//��������߽�ļ��
        const int32_t MARGIN_TOP = 6;	//�������ϱ߽�ļ��

        int idx = 0;
        for (auto a : attrs) {
            //start x, start y
            int32_t sx = x + w, sy = y;
            //�������ұ���ͼ����ұ߾���̫���������ֱ��ضϣ�����д�ڿ�����
            if ((frame.cols - sx) < (maxFontNum*fontSize)) {
                sx = x;
            }

            //����Ŀ�ʼ����x��y
            int x1 = sx + MARGIN_LEFT;
            int y1 = sy + idx * (fontSize + MARGIN_TOP);

            //��������Ӱ����(x1,y1,���������,����߶�)
            cv::Rect rect = cv::Rect(x1, y1, maxFontNum*fontSize, fontSize);
            cv::Mat roi = frame(rect);
            cv::Mat mask = roi.clone();
            cv::rectangle(mask, { 0,0, mask.cols, mask.rows }, CV_RGB(50, 50, 50), -1);
            cv::addWeighted(roi, 0.3, mask, 0.7, 0, roi);

            //��������
            wchar_t msg[200] = { 0 };
            swprintf(msg, 200, L"%hs", a.name.c_str());
            //���������ֵ�������Ϊ����ģ����ԣ�y1���ֵ��ϲ����꣬���������������ƶ�һ���ֵĸ߶�
            text_.putText(frame, msg, cvPoint(x1, y1+ fontSize), color);

            idx++;
        }
    }

protected:
    CvxText text_;
};


}