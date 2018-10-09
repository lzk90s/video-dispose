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

class BikeMixer : public VMixer<algo::BikeObject> {
public:
    BikeMixer() : VMixer("bike") {}

protected:

    void doMixFrame(cv::Mat &frame, vector<algo::BikeObject> &objs1, vector<algo::BikeObject> &objs2) override {
        for (uint32_t idx = 0; idx < objs1.size() && idx < objs2.size(); idx++) {
            // 画矩形框
            algo::Rect &rect = objs1[idx].detect;
            int32_t thickness = 2;
            int32_t x = rect[0], y = rect[1], w=rect[2], h=rect[3];
            cv::rectangle(frame, cvPoint(x, y), cvPoint(x + w, y + h), CV_RGB(0, 0, 255), thickness, 1, 0);

            //如果机动车上有人，则输出人的信息
            if (!objs2[idx].persons.empty()) {
                // 只输出一个人
                for (auto &p : objs2[idx].persons) {
                    // 需要混到流中的属性
                    vector<Attribute> mixableAttrs;
                    mixableAttrs.push_back(algo::Attribute().WithName(getTypeString(objs1[idx].type)));
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

    string getTypeString(algo::ObjectType type) {
        switch (type) {
        case BIKE:
            return "自行车";
            break;
        case MOTOBIKE:
            return "摩托车";
            break;
        case TRICYCLE:
            return "三轮车";
            break;
        default:
            return "";
        }
    }

};

}