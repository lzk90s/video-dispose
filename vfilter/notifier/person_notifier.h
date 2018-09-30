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
    uint8_t *image;
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
    string base64Img;
    Base64::Encode((char*)p.image, &base64Img);
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
    string buildNotifyMsg(cv::Mat &img, algo::PersonObject &obj) override {
        PersonNotifyMsg msg;

        Bgr2JpegConverter converter;
        converter.Convert(img.data, img.cols, img.rows, 100);

        msg.image = (uint8_t*)converter.GetImgBuffer();
        msg.flag = "1";
        msg.channelId = 2;
        msg.type = obj.type;

        msg.sexCodeName = obj.attrs.sex.name;
        msg.ageCodeName = obj.attrs.age.name;
        msg.hairCodeName = obj.attrs.hair.name;
        msg.hatCodeName = obj.attrs.hat.name;
        msg.orientationCodeName = obj.attrs.orientation.name;
        msg.knapsackCodeName = obj.attrs.knapsack.name;
        msg.umbrellaCodeName = obj.attrs.umbrella.name;
        msg.bagCodeName = obj.attrs.bag.name;
        msg.maskCodeName = obj.attrs.mask.name;
        msg.glassesCodeName = obj.attrs.glasses.name;
        msg.trolleyCaseCodeName = obj.attrs.trolleyCase.name;
        msg.barrowCodeName = obj.attrs.barrow.name;
        msg.upperTextureCodeName = obj.attrs.upperTexture.name;
        msg.upperColorCodeName = obj.attrs.upperColor.name;
        msg.bottomColorCodeName = obj.attrs.bottomColor.name;
        msg.upperCodeName = obj.attrs.upperType.name;
        msg.bottomCodeName = obj.attrs.bottomType.name;
        msg.babyCodeName = obj.attrs.baby.name;

        json j = msg;
        return j.dump();
    }


    string getRequestURL() override {
        return "/internal/snap/passerby";
    }
};

}