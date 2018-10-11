#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include "json/json.hpp"

using namespace  std;
using json = nlohmann::json;

namespace algo {
namespace seemmo {
namespace rec {

typedef struct tagLocationPO {
    int32_t Type;
    string GUID;
    string ContextCode;
    vector<int32_t> Rect;
    vector<int32_t> Trail;

    tagLocationPO() {
        Type = 0;
    }
} LocationPO;

void to_json(json& j, const LocationPO& p) {
    j["Type"] = p.Type;
    if (!p.GUID.empty()) {
        j["GUID"] = p.GUID;
    }
    if (!p.ContextCode.empty()) {
        j["ContextCode"] = p.ContextCode;
    }
    if (!p.Rect.empty()) {
        j["Rect"] = p.Rect;
    }
    if (!p.Trail.empty()) {
        j["Trail"] = p.Trail;
    }
}

class RecParamBuilder {
public:
    string Build(vector<LocationPO> &locations) {
        json j = R"(
            {
                "Recognize": {
                    "Feature": {
                        "IsRec": true
                    },
                    "Person": {
                        "IsRec": true
                    },
                    "Vehicle": {
                        "Brand": {
                            "IsRec": true
                        },
                        "Color": {
                            "IsRec": true
                        },
                        "Plate": {
                            "IsRec": true
                        }
                    }
                }
            }
        )"_json;

        // only one
        j["ObjLocations"][0] = locations;

        return j.dump();
    }
private:

};
}
}
}
