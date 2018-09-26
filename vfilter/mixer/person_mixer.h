#pragma once

#include <map>
#include <string>

#include "common/helper/logger.h"
#include "algo/stub/object_type.h"
#include "vfilter/mixer/vmixer.h"

using namespace std;
using namespace algo;

namespace vf {

class PersonMixer : public VMixer {
public:
    void SetObjects(vector<algo::PersonObject> &objs) {
        unique_lock<mutex> lck(mutex_);
        objs_.clear();
        objs_ = objs;
    }

    void MixFrame(cv::Mat &frame) {
        vector<algo::PersonObject> tmpObjs;
        {
            unique_lock<mutex> lck(mutex_);
            tmpObjs = objs_;
        }

        for (auto t : tmpObjs) {
            // 画矩形框
            algo::Rect &rect = t.detect;
            int32_t x = rect[0], y = rect[1], w = rect[2], h = rect[3];
            mixObjectRectangle(frame, x, y, w, h, CV_RGB(255, 0, 0));

            // 需要混到流中的属性
            vector<Attribute> mixableAttrs;
            mixableAttrs.push_back(t.attrs.sex);
            mixableAttrs.push_back(t.attrs.age);
            mixableAttrs.push_back(t.attrs.hair);
            mixableAttrs.push_back(t.attrs.hat);
            mixableAttrs.push_back(t.attrs.upperColor);

            mixObjectAttributeText(frame, x, y, w, h, mixableAttrs);
        }
    }


private:
    vector<algo::PersonObject> objs_;
    mutex mutex_;
};

}