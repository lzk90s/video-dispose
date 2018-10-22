#pragma once

#include "common/helper/base64.h"
#include "common/helper/jpeg_helper.h"

#include "vfilter/notifier/notifier.h"
#include "vfilter/setting.h"

#include "json/json.hpp"

using namespace  std;
using json = nlohmann::json;


namespace vf {

typedef struct tagPersonNotifyMsg {
    uint8_t *image;		//jpeg
    uint64_t imageSize;	//jpeg size
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
} PersonNotifyMsg;

void to_json(json& j, const PersonNotifyMsg& p) {
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

class PersonNotifier : public Notifier<algo::PersonObject> {
public:
    PersonNotifier()
        : Notifier("person") {
    }

protected:
    string buildNotifyMsg(uint32_t channelId, cv::Mat &img, algo::PersonObject &obj) override {
        PersonNotifyMsg msg;

        Bgr2JpegConverter converter;
        converter.Convert(img.data, img.cols, img.rows, 100);

        msg.image = (uint8_t*)converter.GetImgBuffer();
        msg.imageSize = converter.GetSize();
        msg.flag = "1";
        msg.channelId = channelId;
        msg.type = obj.type;

        msg.sexCodeName = obj.attrs[algo::PersonObject::AttrType::SEX].name;
        msg.ageCodeName = obj.attrs[algo::PersonObject::AttrType::AGE].name;
        msg.hairCodeName = obj.attrs[algo::PersonObject::AttrType::HAIR].name;
        msg.hatCodeName = obj.attrs[algo::PersonObject::AttrType::HAT].name;
        msg.orientationCodeName = obj.attrs[algo::PersonObject::AttrType::ORIENTATION].name;
        msg.knapsackCodeName = obj.attrs[algo::PersonObject::AttrType::KNAPSACK].name;
        msg.umbrellaCodeName = obj.attrs[algo::PersonObject::AttrType::UMBERLLA].name;
        msg.bagCodeName = obj.attrs[algo::PersonObject::AttrType::BAG].name;
        msg.maskCodeName = obj.attrs[algo::PersonObject::AttrType::MASK].name;
        msg.glassesCodeName = obj.attrs[algo::PersonObject::AttrType::GLASSES].name;
        msg.trolleyCaseCodeName = obj.attrs[algo::PersonObject::AttrType::TROLLEY_CASE].name;
        msg.barrowCodeName = obj.attrs[algo::PersonObject::AttrType::BARROW].name;
        msg.upperTextureCodeName = obj.attrs[algo::PersonObject::AttrType::UPPER_TEXTURE].name;
        msg.upperColorCodeName = obj.attrs[algo::PersonObject::AttrType::UPPER_COLOR].name;
        msg.bottomColorCodeName = obj.attrs[algo::PersonObject::AttrType::BOTTOM_COLOR].name;
        msg.upperCodeName = obj.attrs[algo::PersonObject::AttrType::UPPER_TYPE].name;
        msg.bottomCodeName = obj.attrs[algo::PersonObject::AttrType::BOTTOM_TYPE].name;
        msg.babyCodeName = obj.attrs[algo::PersonObject::AttrType::BABY].name;

        json j = msg;
        return j.dump();
    }


    string getRequestURL() override {
        return "/internal/snap/passerby";
    }

    bool isInvalidPicture(uint32_t width, uint32_t height) override {
        return (uint32_t)width < GlobalSettings::getInstance().personPictureMinWidth;
    }
};

}