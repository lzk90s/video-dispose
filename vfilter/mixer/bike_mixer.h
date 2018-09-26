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

    void SetObjects(vector<algo::BikeObject> &objs) {
        unique_lock<mutex> lck(mutex_);
        objs_.clear();
        objs_ = objs;
    }

    void MixFrame(cv::Mat &frame) {
        vector<algo::BikeObject> tmpObjs;
        {
            unique_lock<mutex> lck(mutex_);
            tmpObjs = objs_;
        }

        for (auto t : tmpObjs) {
            // »­¾ØÐÎ¿ò
            algo::Rect &rect = t.detect;
            int32_t thickness = 2;
            int32_t x = rect[0], y = rect[1], w=rect[2], h=rect[3];
            cv::rectangle(frame, cvPoint(x, y), cvPoint(x + w, y + h), CV_RGB(0, 0, 255), thickness, 1, 0);

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
    vector<algo::BikeObject> objs_;
    mutex mutex_;
};

}