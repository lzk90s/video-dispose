#pragma once

#include "vfilter/notifier/notifier.h"
#include "vfilter/setting.h"

#include "json/json.hpp"

using namespace  std;
using json = nlohmann::json;


namespace vf {



class BikeNotifier : public Notifier<algo::VehicleObject> {
public:
    BikeNotifier()
        : Notifier("bike", GlobalSettings::getInstance().notifyServerHost + "/bike") {
    }

protected:
    string buildNotifyMsg(cv::Mat &img, algo::VehicleObject &obj) override {
        return "";
    }
};

}