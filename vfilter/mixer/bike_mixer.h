#pragma once

#include <map>
#include <string>
#include <mutex>

#include "common/helper/logger.h"
#include "algo/stub/object_type.h"
#include "vfilter/mixer/vmixer.h"

using namespace std;
using namespace algo;

namespace vf {
class BikeMixer : public VMixer {
public:

    void SetDetectedObjects(vector<algo::BikeObject> &objs) {
        unique_lock<mutex> lck(mutex_);
        detect_.clear();
        for (auto o : objs) {
            detect_[o.guid] = o;
        }
    }

    void SetRecognizedObjects(vector<algo::BikeObject> &objs) {
        unique_lock<mutex> lck(mutex_);
        for (auto o : objs) {
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
            cv::rectangle(frame, cvPoint(rect.x, rect.y), cvPoint(rect.x + rect.w, rect.y + rect.h), CV_RGB(0, 0, 255), thickness,
                          1, 0);

//             if (recog_.find(t.first) != recog_.end()) {
//                 auto persons = recog_[t.first].persons;
//                 int idx = 0;
//                 for (auto p : persons) {
//                     for (auto a : p.attrs) {
//                         if (!a.visable) {
//                             continue;
//                         }
//                         // Ð´ÊôÐÔ
//                         int x = rect.x + rect.w + 2;
//                         int y = rect.y + (idx * DEFAULT_FONT_SIZE) + 2;
//                         std::wstring msg = StringConv::StringToWString(a.value);
//                         text_.putText(frame, msg.c_str(), cvPoint(x, y), CV_RGB(255, 0, 0));
//                     }
//                 }
//             }
        }
    }

private:
    typedef map<string, algo::BikeObject> ObjectMap;
    ObjectMap detect_;		// ¼ì²â
    ObjectMap recog_;		// Ê¶±ð
    mutex mutex_;
};

}