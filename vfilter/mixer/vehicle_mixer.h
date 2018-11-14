#pragma once

#include <map>
#include <string>

#include "algo/stub/object_type.h"
#include "vfilter/mixer/vmixer.h"

using namespace std;
using namespace algo;

namespace vf {
class VehicleMixer : public VMixer<algo::VehicleObject> {
public:
    VehicleMixer() : VMixer("vehicle") {}

protected:
    void doMixFrame(cv::Mat &frame, vector<algo::VehicleObject> &objs1, vector<algo::VehicleObject> &objs2) override {

        for (uint32_t idx = 0; idx < objs1.size() && idx < objs2.size(); idx++) {
            // 区域从objs1取
            algo::Rect &rect = objs1[idx].detect;
            int32_t x = rect[0], y = rect[1], w = rect[2], h = rect[3];
            mixObjectRectangle(frame, x, y, w, h, CV_RGB(0, 255, 0));

            // 属性从objs2取
            vector<algo::Attribute> mixableAttrs;

            algo::Attributes &attrs = objs2[idx].attrs;
            mixableAttrs.push_back(algo::Attribute().WithName(getTypeString(objs1[idx].type)));
            mixableAttrs.push_back(attrs[algo::VehicleObject::AttrType::BRAND]);
            mixableAttrs.push_back(attrs[algo::VehicleObject::AttrType::COLOR]);
            mixableAttrs.push_back(attrs[algo::VehicleObject::AttrType::PLATE]);
            mixableAttrs.push_back(attrs[algo::VehicleObject::AttrType::TYPE]);

            mixObjectAttributeText(frame, x, y, w, h, mixableAttrs);
        }
    }

    string getTypeString(algo::ObjectType type) {
        switch (type) {
        case BUS:
            return "公交车";
            break;
        case CAR:
            return "轿车";
            break;
        case VAN:
            return "面包车";
            break;
        case TRUCK:
            return "卡车";
            break;
        default:
            return "";
        }
    }

    bool textRightForce() override {
        return false;
    }
};

}