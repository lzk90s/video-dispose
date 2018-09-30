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
            msg.sexCodeName = p.attrs.sex.name;
            msg.ageCodeName = p.attrs.age.name;
            msg.hairCodeName = p.attrs.hair.name;
            msg.hatCodeName = p.attrs.hat.name;
            msg.orientationCodeName = p.attrs.orientation.name;
            msg.knapsackCodeName = p.attrs.knapsack.name;
            msg.umbrellaCodeName = p.attrs.umbrella.name;
            msg.bagCodeName = p.attrs.bag.name;
            msg.maskCodeName = p.attrs.mask.name;
            msg.glassesCodeName = p.attrs.glasses.name;
            msg.trolleyCaseCodeName = p.attrs.trolleyCase.name;
            msg.barrowCodeName = p.attrs.barrow.name;
            msg.upperTextureCodeName = p.attrs.upperTexture.name;
            msg.upperColorCodeName = p.attrs.upperColor.name;
            msg.bottomColorCodeName = p.attrs.bottomColor.name;
            msg.upperCodeName = p.attrs.upperType.name;
            msg.bottomCodeName = p.attrs.bottomType.name;
            msg.babyCodeName = p.attrs.baby.name;

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