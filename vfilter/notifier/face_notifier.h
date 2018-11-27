#pragma once


#include "common/helper/base64.h"
#include "common/helper/jpeg_helper.h"

#include "vfilter/notifier/notifier.h"
#include "vfilter/config/setting.h"

#include "json/json.hpp"

using namespace  std;
using json = nlohmann::json;


namespace vf {

typedef struct tagFaceNotifyMsg {
    uint8_t *image;		//jpeg
    uint64_t imageSize;	//jpeg size
    string flag;
    int32_t channelId;
    int32_t type;
} FaceNotifyMsg;

void to_json(json& j, const FaceNotifyMsg& p) {
    string img((char*)p.image, p.imageSize);
    string base64Img;
    Base64::Encode(img, &base64Img);

    j["image"] = base64Img;
    j["flag"] = p.flag;
    j["channelId"] = p.channelId;
    j["type"] = p.type;
}


class FaceNotifier : public Notifier<algo::FaceObject> {
public:
    FaceNotifier()
        : Notifier("face") {
    }

protected:
    string buildNotifyMsg(uint32_t channelId, cv::Mat &img, algo::FaceObject &obj) override {
        FaceNotifyMsg msg;

        Bgr2JpegConverter converter;
        converter.Convert(img.data, img.cols, img.rows, 100);

        msg.image = (uint8_t*)converter.GetImgBuffer();
        msg.imageSize = converter.GetSize();
        msg.flag = "0";
        msg.channelId = channelId;
        msg.type = obj.type;

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
        return "/internal/snap/face";
    }

    bool isInvalidPicture(uint32_t width, uint32_t height) override {
        return (uint32_t)width < G_CFG().facePictureMinWidth;
    }
};

}