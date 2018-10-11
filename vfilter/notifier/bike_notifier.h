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
            msg.sexCodeName = p.attrs[algo::PersonObject::AttrType::SEX].name;
            msg.ageCodeName = p.attrs[algo::PersonObject::AttrType::AGE].name;
            msg.hairCodeName = p.attrs[algo::PersonObject::AttrType::HAIR].name;
            msg.hatCodeName = p.attrs[algo::PersonObject::AttrType::HAT].name;
            msg.orientationCodeName = p.attrs[algo::PersonObject::AttrType::ORIENTATION].name;
            msg.knapsackCodeName = p.attrs[algo::PersonObject::AttrType::KNAPSACK].name;
            msg.umbrellaCodeName = p.attrs[algo::PersonObject::AttrType::UMBERLLA].name;
            msg.bagCodeName = p.attrs[algo::PersonObject::AttrType::BAG].name;
            msg.maskCodeName = p.attrs[algo::PersonObject::AttrType::MASK].name;
            msg.glassesCodeName = p.attrs[algo::PersonObject::AttrType::GLASSES].name;
            msg.trolleyCaseCodeName = p.attrs[algo::PersonObject::AttrType::TROLLEY_CASE].name;
            msg.barrowCodeName = p.attrs[algo::PersonObject::AttrType::BARROW].name;
            msg.upperTextureCodeName = p.attrs[algo::PersonObject::AttrType::UPPER_TEXTURE].name;
            msg.upperColorCodeName = p.attrs[algo::PersonObject::AttrType::UPPER_COLOR].name;
            msg.bottomColorCodeName = p.attrs[algo::PersonObject::AttrType::BOTTOM_COLOR].name;
            msg.upperCodeName = p.attrs[algo::PersonObject::AttrType::UPPER_TYPE].name;
            msg.bottomCodeName = p.attrs[algo::PersonObject::AttrType::BOTTOM_TYPE].name;
            msg.babyCodeName = p.attrs[algo::PersonObject::AttrType::BABY].name;

            // only one
            break;
        }

        json j = msg;
        return j.dump();
    }

    string getRequestURL() override {
        return "/internal/snap/passerby";
    }
};

}