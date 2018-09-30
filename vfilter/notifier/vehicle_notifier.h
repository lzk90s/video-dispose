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
    string base64Img;
    Base64::Encode((char*)p.image, &base64Img);
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
    string buildNotifyMsg(cv::Mat &img, algo::VehicleObject &obj) override {
        VehicleNotifyMsg msg;

        Bgr2JpegConverter converter;
        converter.Convert(img.data, img.cols, img.rows, 100);

        msg.image = (uint8_t*)converter.GetImgBuffer();
        msg.flag = "2";
        msg.channelId = 2;
        msg.type = obj.type;

        msg.plateLicence = obj.attrs.plate.name;
        msg.carPatternName = obj.attrs.type.name;
        msg.plateTypeCodeName = "";
        msg.colorCodeName = obj.attrs.color.name;
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
};

}