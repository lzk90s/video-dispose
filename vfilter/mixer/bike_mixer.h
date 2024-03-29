#pragma once

#include <map>
#include <string>

#include "algo/stub/object_type.h"
#include "vfilter/mixer/vmixer.h"

namespace video {
namespace filter {

class BikeMixer : public VMixer<algo::BikeObject> {
public:
    BikeMixer() : VMixer("bike") {}

protected:

    void doMixFrame(cv::Mat &frame, std::vector<algo::BikeObject> &objs1, std::vector<algo::BikeObject> &objs2) override {
        for (uint32_t idx = 0; idx < objs1.size() && idx < objs2.size(); idx++) {
            algo::Rect &rect = objs1[idx].detect;
            int32_t x = rect[0], y = rect[1], w = rect[2], h = rect[3];
            mixObjectRectangle(frame, x, y, w, h, CV_RGB(255, 255, 0));

            //如果机动车上有人，则输出人的信息
            if (!objs2[idx].persons.empty()) {
                // 只输出一个人
                for (auto &p : objs2[idx].persons) {
                    // 需要混到流中的属性
                    std::vector<algo::Attribute> mixableAttrs;

                    algo::Attributes &attrs = p.attrs;
                    mixableAttrs.push_back(algo::Attribute().WithName(getTypeString(objs1[idx].type)));
                    mixableAttrs.push_back(attrs[algo::PersonObject::AttrType::SEX]);
                    mixableAttrs.push_back(attrs[algo::PersonObject::AttrType::AGE]);
                    mixableAttrs.push_back(attrs[algo::PersonObject::AttrType::HAIR]);
                    mixableAttrs.push_back(attrs[algo::PersonObject::AttrType::HAT]);
                    mixableAttrs.push_back(attrs[algo::PersonObject::AttrType::UPPER_COLOR]);

                    mixObjectAttributeText(frame, x, y, w, h, mixableAttrs);
                    break;
                }
            }
        }
    }

    std::string getTypeString(algo::ObjectType type) {
        switch (type) {
        case algo::BIKE:
            return "自行车";
            break;
        case algo::MOTOBIKE:
            return "摩托车";
            break;
        case algo::TRICYCLE:
            return "三轮车";
            break;
        default:
            return "";
        }
    }

};

}
}