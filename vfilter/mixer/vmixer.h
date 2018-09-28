#pragma once

#include <string>
#include <map>
#include <mutex>
#include <vector>

#include "opencv/cv.h"
#include "opencv2/opencv.hpp"
#include "vfilter/mixer/cvx_text.h"
#include "vfilter/mixer/cvx_text.h"
#include "algo/stub/object_type.h"

using namespace std;

namespace vf {
static const char *DEFAULT_FONT = "/usr/share/fonts/truetype/simsun.ttf";
static const int FONT_MIN_SIZE = 18;	//������С��
static const int FONT_MAX_SIZE = 40;	//��������

template<class T>
class VMixer {
public:
    VMixer(const string &type)
        : text_(DEFAULT_FONT),
          type_(type) {
        cv::Scalar size{ FONT_MIN_SIZE, 0.5, 0.1, 0 };
        text_.setFont(nullptr, &size, nullptr, nullptr);
    }

    //����Ŀ��
    virtual void SetDetectedObjects(vector<T> &objs) {
        unique_lock<mutex> lck(mutex_);

        for (auto &o : objs) {
            //���û���ҵ�������ӵ��Ѵ���Ŀ��������
            if (existObjs_.find(o.guid) == existObjs_.end()) {
                ObjectWithCounter newObj;
                newObj.obj1 = o;
                existObjs_[o.guid] = newObj;
            } else {
                existObjs_[o.guid].obj1 = o;
            }
        }

        vector<string> disappearedObjs;
        for (auto &o : existObjs_) {
            bool exist = false;
            for (auto &i : objs) {
                if (i.guid == o.second.obj1.guid) {
                    exist = true;
                    break;
                }
            }
            if (!exist) {
                /*
                * �����µ�Ŀ���в����ڣ����������
                * 1. Ŀ���Ѿ���ȫ��ʧ
                * 2. Ŀ��ֻ���ڸպ���һ֡��û�м�⵽����������Ŀ���Ѿ���ʧ��
                * ��������������ͨ������Ŀ��ļ�������Ŀ�����Ϊ0��ʱ�򣬱�ʾĿ�������ʧ�ˡ���ɾ����
                */
                o.second.cnt = o.second.cnt-1;
                if (o.second.cnt == 0) {
                    disappearedObjs.push_back(o.first);
                }
            }
        }

        //ɾ����ʧ��Ŀ��
        for (auto &o : disappearedObjs) {
            existObjs_.erase(o);
        }
    }

    virtual void SetRecognizedObjects(vector<T> &objs) {
        unique_lock<mutex> lck(mutex_);
        for (auto o : objs) {
            if (existObjs_.find(o.guid) != existObjs_.end()) {
                existObjs_[o.guid].obj2 = o;
            }
        }
    }

    virtual void MixFrame(cv::Mat &frame) {
        vector<T> tmpObjs1, tmpObjs2;
        {
            unique_lock<mutex> lck(mutex_);
            for (auto o : existObjs_) {
                tmpObjs1.push_back(o.second.obj1);
                tmpObjs2.push_back(o.second.obj2);
            }
        }
        doMixFrame(frame, tmpObjs1, tmpObjs2);
    }

protected:
    virtual void doMixFrame(cv::Mat &frame, vector<T> &objs1, vector<T> &objs2) {

    }

    //��Ŀ��ľ��ο�
    virtual void mixObjectRectangle(cv::Mat &frame, int32_t x, int32_t y, int32_t w, int32_t h,
                                    CvScalar color= CV_RGB(255,255,255)) {
        int32_t thickness = 3;
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
        uint32_t maxFontNum = 0;
        for (auto &a : attrs) {
            wchar_t msg[200] = { 0 };
            swprintf(msg, 200, L"%hs", a.name.c_str());
            if (wcslen(msg) > maxFontNum) {
                maxFontNum = wcslen(msg);
            }
        }

        const int32_t MARGIN_LEFT = 8;	//��������߽�ļ��
        const int32_t MARGIN_TOP = 6;	//�������ϱ߽�ļ��

        int idx = 0;
        for (auto &a : attrs) {
            if (a.name.empty()) {
                continue;
            }

            //start x, start y
            int32_t sx = x + w, sy = y;
            //�������ұ���ͼ����ұ߾���̫���������ֱ��ضϣ�����д�ڿ�����
            if ((uint32_t)(frame.cols - sx) < (maxFontNum*fontSize)) {
                sx = x;
            }

            //����Ŀ�ʼ����x��y
            int x1 = sx + MARGIN_LEFT;
            int y1 = sy + idx * (fontSize + MARGIN_TOP);
            int w1 = maxFontNum * fontSize;
            int h1 = fontSize;

            //�ضϴ���
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

            // ������󣬲���ʾ
            if (y1 + fontSize > frame.rows) {
                LOG_WARN("Ignore attribute {}", a.name.c_str());
                continue;
            }

            //��������Ӱ����(x1,y1,���������,����߶�)
            cv::Rect rect = cv::Rect(x1, y1, w1, h1);
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

    class ObjectWithCounter {
        const static int32_t INIT_OBJECT_DISAPPEAR_COUNT = 10;	//Ŀ����ʧ��ʼ����
        typedef int32_t OjbectDisappearCounter;

    public:
        T obj1;		//obj1 �洢�����
        T obj2;		//obj2 �洢ʶ����
        OjbectDisappearCounter cnt;

        ObjectWithCounter() {
            cnt = INIT_OBJECT_DISAPPEAR_COUNT;
        }
    };

    CvxText text_;
    string type_;
    mutex mutex_;
    //�Ѿ����ڵ�Ŀ��
    map<string, ObjectWithCounter> existObjs_;
};


}