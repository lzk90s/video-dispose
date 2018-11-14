#pragma once

#include "common/helper/base64.h"
#include "common/helper/jpeg_helper.h"

#include "vfilter/notifier/notifier.h"
#include "vfilter/setting.h"

#include "json/json.hpp"

using namespace  std;
using json = nlohmann::json;


namespace vf {

typedef struct tagBikeNotifyMsg {
    uint8_t *image;
    uint64_t imageSize;
    string flag;
    int32_t channelId;
    int32_t type;

    string sexCodeName;
    string ageCodeName;
    string hairCodeName;
    string hatCodeName;
    string orientationCodeName;
    string knapsackCodeName;
    string umbrellaCodeName;
    string bagCodeName;
    string maskCodeName;
    string glassesCodeName;
    string trolleyCaseCodeName;
    string barrowCodeName;
    string upperTextureCodeName;
    string upperColorCodeName;
    string bottomColorCodeName;
    string upperCodeName;
    string bottomCodeName;
    string babyCodeName;
} BikeNotifyMsg;

void to_json(json& j, const BikeNotifyMsg& p) {
    string img((char*)p.image, p.imageSize);
    string base64Img;
    Base64::Encode(img, &base64Img);

    j["image"] = base64Img;
    j["flag"] = p.flag;
    j["channelId"] = p.channelId;
    j["type"] = p.type;

    j["sexCodeName"] = p.sexCodeName;
    j["ageCodeName"] = p.ageCodeName;
    j["hairCodeName"] = p.hairCodeName;
    j["hatCodeName"] = p.hatCodeName;
    j["orientationCodeName"] = p.orientationCodeName;
    j["knapsackCodeName"] = p.knapsackCodeName;
    j["umbrellaCodeName"] = p.umbrellaCodeName;
    j["bagCodeName"] = p.bagCodeName;
    j["maskCodeName"] = p.maskCodeName;
    j["glassesCodeName"] = p.glassesCodeName;
    j["trolleyCaseCodeName"] = p.trolleyCaseCodeName;
    j["barrowCodeName"] = p.barrowCodeName;
    j["upperTextureCodeName"] = p.upperTextureCodeName;
    j["upperColorCodeName"] = p.upperColorCodeName;
    j["bottomColorCodeName"] = p.bottomColorCodeName;
    j["upperCodeName"] = p.upperCodeName;
    j["bottomCodeName"] = p.bottomCodeName;
    j["babyCodeName"] = p.babyCodeName;
}


class BikeNotifier : public Notifier<algo::BikeObject> {
public:
    BikeNotifier()
        : Notifier("bike") {
    }

protected:
    string buildNotifyMsg(uint32_t channelId, cv::Mat &img, algo::BikeObject &obj) override {
        BikeNotifyMsg msg;

        Bgr2JpegConverter converter;
        converter.Convert(img.data, img.cols, img.rows, 100);

        msg.image = (uint8_t*)converter.GetImgBuffer();
        msg.imageSize = converter.GetSize();
        msg.flag = "1";
        msg.channelId = channelId;
        msg.type = obj.type;

        //非机动车显示其上人的属性
        for (auto &p : obj.persons) {
            using AttrType = algo::PersonObject::AttrType;
            msg.sexCodeName = getAttrName(p.attrs, AttrType::SEX);
            msg.ageCodeName = getAttrName(p.attrs, AttrType::AGE);
            msg.hairCodeName = getAttrName(p.attrs, AttrType::HAIR);
            msg.hatCodeName = getAttrName(p.attrs, AttrType::HAT);
            msg.orientationCodeName = getAttrName(p.attrs, AttrType::ORIENTATION);
            msg.knapsackCodeName = getAttrName(p.attrs, AttrType::KNAPSACK);
            msg.umbrellaCodeName = getAttrName(p.attrs, AttrType::UMBERLLA);
            msg.bagCodeName = getAttrName(p.attrs, AttrType::BAG);
            msg.maskCodeName = getAttrName(p.attrs, AttrType::MASK);
            msg.glassesCodeName = getAttrName(p.attrs, AttrType::GLASSES);
            msg.trolleyCaseCodeName = getAttrName(p.attrs, AttrType::TROLLEY_CASE);
            msg.barrowCodeName = getAttrName(p.attrs, AttrType::BARROW);
            msg.upperTextureCodeName = getAttrName(p.attrs, AttrType::UPPER_TEXTURE);
            msg.upperColorCodeName = getAttrName(p.attrs, AttrType::UPPER_COLOR);
            msg.bottomColorCodeName = getAttrName(p.attrs, AttrType::BOTTOM_COLOR);
            msg.upperCodeName = getAttrName(p.attrs, AttrType::UPPER_TYPE);
            msg.bottomCodeName = getAttrName(p.attrs, AttrType::BOTTOM_TYPE);
            msg.babyCodeName = getAttrName(p.attrs, AttrType::BABY);

            // only one
            break;
        }

        json j = msg;
        return j.dump();
    }

    string getAttrName(algo::Attributes &attrs, algo::PersonObject::AttrType type) {
        if (attrs.find(type) != attrs.end()) {
            return attrs[type].name;
        } else {
            return "";
        }
    }

    string getRequestURL() override {
        return "/internal/snap/passerby";
    }

    bool isInvalidPicture(uint32_t width, uint32_t height) override {
        return (uint32_t)width < GlobalSettings::getInstance().bikePictureMinWidth;
    }
};

}