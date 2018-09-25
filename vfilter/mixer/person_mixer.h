#pragma once

#include <map>
#include <string>

#include "algo/stub/object_type.h"
#include "vfilter/mixer/vmixer.h"

using namespace std;

namespace vf {
class PersonMixer : public VMixer {
public:
    void SetDetectedObjects(vector<algo::PersonObject> &objs) {
        unique_lock<mutex> lck(mutex_);
        detect_.clear();
        for (auto o : objs) {
            LOG_INFO("DDD: {}", o.guid);
            detect_[o.guid] = o;
        }
    }

    void SetRecognizedObjects(vector<algo::PersonObject> &objs) {
        unique_lock<mutex> lck(mutex_);
        for (auto o : objs) {
            bool f = true;
            if (detect_.find(o.guid) == detect_.end()) {
                for (auto s : detect_) {
                    LOG_INFO("SSS : {}", s.second.guid);
                }
                f = false;
            }
            LOG_INFO("---KKKKKKKKKKK {}, {} {}", o.guid, f, detect_.size());
            recog_[o.guid] = o;
        }
    }

    void MixFrame(cv::Mat &frame) {
        ObjectMap tmpDetect, tmpRecog;
        {
            unique_lock<mutex> lck(mutex_);
            tmpDetect = detect_;
            tmpRecog = recog_;
        }

        for (auto t : tmpDetect) {
            // »­¾ØÐÎ¿ò
            algo::Rect &rect = t.second.detect;
            int32_t thickness = 2;
            cv::rectangle(frame, cvPoint(rect.x, rect.y), cvPoint(rect.x + rect.w, rect.y + rect.h), CV_RGB(255, 0, 0), thickness,
                          1, 0);

            if (recog_.find(t.first) != recog_.end()) {
                vector<Attribute> mixableAttrs;
                LOG_INFO("--------444444444--- EXIST");
                mixableAttrs.push_back(t.second.attrs.sex);
                mixableAttrs.push_back(t.second.attrs.age);
                mixableAttrs.push_back(t.second.attrs.hair);

                int idx = 0;
                for (auto a : mixableAttrs) {
                    // Ð´ÊôÐÔ
                    int x = rect.x + rect.w + 2;
                    int y = rect.y + (idx * DEFAULT_FONT_SIZE) + 2;
                    std::wstring msg = StringConv::StringToWString(a.name);
                    text_.putText(frame, msg.c_str(), cvPoint(x, y), CV_RGB(255, 0, 0));
                }
            } else {
                //LOG_INFO("--------33333333--- NOT EXIST {}", t.first);
            }
        }
    }


private:
    typedef map<string, algo::PersonObject> ObjectMap;
    ObjectMap detect_;
    ObjectMap recog_;
    mutex mutex_;
};

}