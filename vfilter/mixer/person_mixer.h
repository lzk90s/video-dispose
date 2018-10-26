#pragma once

#include <map>
#include <string>

#include "common/helper/logger.h"
#include "algo/stub/object_type.h"
#include "vfilter/mixer/vmixer.h"

using namespace std;
using namespace algo;

namespace vf {

class PersonMixer : public VMixer<algo::PersonObject> {
public:
    PersonMixer() : VMixer("person") {}

protected:
    void doMixFrame(cv::Mat &frame, vector<algo::PersonObject> &objs1, vector<algo::PersonObject> &objs2) override {
        for (uint32_t idx = 0; idx < objs1.size() && idx < objs2.size(); idx++) {
            algo::Rect rect = recoveryObjectRect(objs1[idx].detect, objs1[idx].trail);
            int32_t x = rect[0], y = rect[1], w = rect[2], h = rect[3];
            mixObjectRectangle(frame, x, y, w, h, CV_RGB(255, 0, 0));

            // 需要混到流中的属性
            vector<Attribute> mixableAttrs;
            mixableAttrs.push_back(algo::Attribute().WithName(getTypeString(objs1[idx].type)));
            mixableAttrs.push_back(objs2[idx].attrs[algo::PersonObject::AttrType::SEX]);
            mixableAttrs.push_back(objs2[idx].attrs[algo::PersonObject::AttrType::AGE]);
            mixableAttrs.push_back(objs2[idx].attrs[algo::PersonObject::AttrType::HAIR]);
            mixableAttrs.push_back(objs2[idx].attrs[algo::PersonObject::AttrType::HAT]);
            mixableAttrs.push_back(objs2[idx].attrs[algo::PersonObject::AttrType::UPPER_COLOR]);

            mixObjectAttributeText(frame, x, y, w, h, mixableAttrs);
        }
    }

    string getTypeString(algo::ObjectType type) {
        return "行人";
    }
};

}