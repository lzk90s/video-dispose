#pragma once

#include <cstdint>
#include "vfilter/notifier/notifier.h"
#include "vfilter/config/setting.h"

#include "json/json.hpp"

using json = nlohmann::json;

namespace video {
namespace filter {

typedef struct tagVehicleNotifyMsg {
    uint8_t *image;
    uint64_t imageSize;
    std::string flag;
    int32_t channelId;
    int32_t type;

    std::string plateLicence;
    std::string carPatternName;
    std::string plateTypeCodeName;
    std::string colorCodeName;
    std::string coDriverPersonCodeName;
    std::string callCodeName;
    std::string rackCodeName;
    std::string spareTireCodeName;
    std::string sunroofCodeName;
    std::string dangerCodeName;
    std::string mainDriverBeltCodeName;
    std::string coDriverBeltCodeName;
} VehicleNotifyMsg;

void to_json(json& j, const VehicleNotifyMsg& p) {
    std::string img((char*)p.image, p.imageSize);
    std::string base64Img;
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
    std::string buildNotifyMsg(uint32_t channelId, cv::Mat &img, algo::VehicleObject &obj) override {
        VehicleNotifyMsg msg;

        Bgr2JpegConverter converter;
        converter.Convert(img.data, img.cols, img.rows, 100);

        msg.image = (uint8_t*)converter.GetImgBuffer();
        msg.imageSize = converter.GetSize();
        msg.flag = "2";
        msg.channelId = channelId;
        msg.type = obj.type;

        using AttrType = algo::VehicleObject::AttrType;
        msg.plateLicence = getAttrName(obj.attrs, AttrType::PLATE);
        msg.carPatternName = getAttrName(obj.attrs, AttrType::BRAND);
        msg.plateTypeCodeName = getAttrName(obj.attrs, AttrType::TYPE);
        msg.colorCodeName = getAttrName(obj.attrs, AttrType::COLOR);
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

    std::string getAttrName(algo::Attributes &attrs, algo::VehicleObject::AttrType type) {
        if (attrs.find(type) != attrs.end()) {
            return attrs[type].name;
        } else {
            return "";
        }
    }

    std::string getRequestURL() override {
        return "/internal/snap/car";
    }

    bool isInvalidPicture(uint32_t width, uint32_t height) override {
        return (uint32_t)width < G_CFG().vehiclePictureMinWidth;
    }
};

}
}