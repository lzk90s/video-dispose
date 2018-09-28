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
            // 画矩形框
            algo::Rect &rect = t.detect;
            int32_t thickness = 2;
            int32_t x = rect[0], y = rect[1], w=rect[2], h=rect[3];
            cv::rectangle(frame, cvPoint(x, y), cvPoint(x + w, y + h), CV_RGB(0, 0, 255), thickness, 1, 0);

            //如果机动车上有人，则输出人的信息
            if (!t.persons.empty()) {
                // 只输出一个人
                for (auto p : t.persons) {
                    // 需要混到流中的属性
                    vector<Attribute> mixableAttrs;
                    mixableAttrs.push_back(p.attrs.sex);
                    mixableAttrs.push_back(p.attrs.age);
                    mixableAttrs.push_back(p.attrs.hair);
                    mixableAttrs.push_back(p.attrs.hat);
                    mixableAttrs.push_back(p.attrs.upperColor);

                    mixObjectAttributeText(frame, x, y, w, h, mixableAttrs);
                    break;
                }
            }
        }
    }

private:
    vector<algo::BikeObject> objs_;
    mutex mutex_;
};

}