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

typedef struct tagRecLocation {
    int32_t Type;
    string GUID;
    string ContextCode;
    vector<int32_t> Rect;
    vector<int32_t> Trail;
} RecLocation;

void to_json(json& j, const RecLocation& p) {
    j = json{ { "Type", p.Type },{ "GUID", p.GUID },{ "ContextCode", p.ContextCode },{ "Rect", p.Rect },{ "Trail", p.Trail } };
}

class RecParamBuilder {
public:
    string Build(vector<RecLocation> &locations) {
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
