#pragma once

#include "vfilter/notifier/notifier.h"
#include "vfilter/setting.h"

#include "json/json.hpp"

using namespace  std;
using json = nlohmann::json;


namespace vf {

typedef struct tagPersonNotifyMsg {
    char *image;
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

}

class PersonNotifier : public Notifier<algo::VehicleObject> {
public:
    PersonNotifier()
        : Notifier("person", GlobalSettings::getInstance().notifyServerHost + "/person") {
    }

protected:
    string buildNotifyMsg(cv::Mat &img, algo::VehicleObject &obj) override {
        return "";
    }
};

}