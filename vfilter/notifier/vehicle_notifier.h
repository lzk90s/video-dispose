#pragma once

#include <cstdint>
#include "vfilter/notifier/notifier.h"
#include "vfilter/setting.h"

#include "json/json.hpp"

using namespace  std;
using json = nlohmann::json;

namespace vf {

typedef struct tagVehicleNotifyMsg {
    uint8_t *image;
    uint64_t imageSize;
    string flag;
    int32_t channelId;
    int32_t type;

    string plateLicence;
    string carPatternName;
    string plateTypeCodeName;
    string colorCodeName;
    string coDriverPersonCodeName;
    string callCodeName;
    string rackCodeName;
    string spareTireCodeName;
    string sunroofCodeName;
    string dangerCodeName;
    string mainDriverBeltCodeName;
    string coDriverBeltCodeName;
} VehicleNotifyMsg;

void to_json(json& j, const VehicleNotifyMsg& p) {
    string img((char*)p.image, p.imageSize);
    string base64Img;
    Base64::Encode(img, &base64Img);

    j["image"] = base64Img;
    j["flag"] = p.flag;
    j["channelId"] = p.channelId;
    j["type"] = p.type;

    j["plateLicence"] = p.plateLicence;
    j["carPatternName"] = p.carPatternName;
    j["plateTypeCodeName"] = p.plateTypeCodeName;
    j["colorCodeName"] = p.colorCodeName;
    j["coDriverPersonCodeName"] = p.coDriverPersonCodeName;
    j["callCodeName"] = p.callCodeName;
    j["rackCodeName"] = p.rackCodeName;
    j["spareTireCodeName"] = p.spareTireCodeName;
    j["sunroofCodeName"] = p.sunroofCodeName;
    j["dangerCodeName"] = p.dangerCodeName;
    j["mainDriverBeltCodeName"] = p.mainDriverBeltCodeName;
    j["coDriverBeltCodeName"] = p.coDriverBeltCodeName;
}

class VehicleNotifier : public Notifier<algo::VehicleObject> {
public:
    VehicleNotifier()
        : Notifier("vehicle") {
    }

protected:
    string buildNotifyMsg(uint32_t channelId, cv::Mat &img, algo::VehicleObject &obj) override {
        VehicleNotifyMsg msg;

        Bgr2JpegConverter converter;
        converter.Convert(img.data, img.cols, img.rows, 100);

        msg.image = (uint8_t*)converter.GetImgBuffer();
        msg.imageSize = converter.GetSize();
        msg.flag = "2";
        msg.channelId = channelId;
        msg.type = obj.type;

        msg.plateLicence = obj.attrs[algo::VehicleObject::AttrType::PLATE].name;
        msg.carPatternName = obj.attrs[algo::VehicleObject::AttrType::BRAND].name;
        msg.plateTypeCodeName = obj.attrs[algo::VehicleObject::AttrType::TYPE].name;
        msg.colorCodeName = obj.attrs[algo::VehicleObject::AttrType::COLOR].name;
        msg.coDriverPersonCodeName = "";
        msg.callCodeName = "";
        msg.rackCodeName = "";
        msg.spareTireCodeName = "";
        msg.sunroofCodeName = "";
        msg.dangerCodeName = "";
        msg.mainDriverBeltCodeName = "";
        msg.coDriverBeltCodeName = "";

        json j = msg;
        return j.dump();
    }

    string getRequestURL() override {
        return "/internal/snap/car";
    }

    bool isInvalidPicture(uint32_t width, uint32_t height) override {
        return (uint32_t)width < GlobalSettings::getInstance().vehiclePictureMinWidth;
    }
};

}