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

}

class VehicleNotifier : public Notifier<algo::VehicleObject> {
public:
    VehicleNotifier()
        : Notifier("vehicle", GlobalSettings::getInstance().notifyServerHost + "/face") {
    }

protected:
    string buildNotifyMsg(cv::Mat &img, algo::VehicleObject &obj) override {
        return "";
    }

private:
    string buildAttributeJson() {

    }
};

}