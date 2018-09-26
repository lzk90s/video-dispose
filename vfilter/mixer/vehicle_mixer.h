#pragma once

#include <map>
#include <string>

#include "common/helper/logger.h"
#include "algo/stub/object_type.h"
#include "vfilter/mixer/vmixer.h"

using namespace std;
using namespace algo;

namespace vf {
class VehicleMixer : public VMixer {
public:
    void SetObjects(vector<algo::VehicleObject> &objs) {
        unique_lock<mutex> lck(mutex_);
        objs_.clear();
        objs_ = objs;
    }

    void MixFrame(cv::Mat &frame) {
        vector<algo::VehicleObject> tmpOjbs;
        {
            unique_lock<mutex> lck(mutex_);
            tmpOjbs = objs_;
        }

        for (auto t : tmpOjbs) {
            algo::Rect &rect = t.detect;
            int32_t x = rect[0], y = rect[1], w = rect[2], h = rect[3];
            mixObjectRectangle(frame, x, y, w, h, CV_RGB(0, 255, 0));

            vector<Attribute> mixableAttrs;
            mixableAttrs.push_back(t.attrs.brand);
            mixableAttrs.push_back(t.attrs.color);
            mixableAttrs.push_back(t.attrs.plate);
            mixableAttrs.push_back(t.attrs.type);

            mixObjectAttributeText(frame, x, y, w, h, mixableAttrs);
        }
    }


private:
    vector<algo::VehicleObject> objs_;
    mutex mutex_;
};

}