#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include "json/json.hpp"


namespace video {
namespace algo {
namespace seemmo {
namespace rec {

using json = nlohmann::json;

typedef struct tagLocationPO {
    int32_t Type;
    std::string GUID;
    std::string ContextCode;
    std::vector<int32_t> Rect;
    std::vector<int32_t> Trail;

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
    std::string Build(std::vector<LocationPO> &locations) {
        json j = R"(
		 {
			"Detect": {
				"IsDet": true,
				"Threshold": 1,
				"Mode": 0
			},
			"Recognize": {
				"Feature": {
					"IsRec": false
				},
				"Person": {
					"IsRec": true
				},
				"Vehicle": {
					"Brand": {
						"IsRec": true
					},
					"Plate": {
						"IsRec": true
					},
					"Color": {
						"IsRec": true
					},
					"Marker": {
						"IsRec": true
					},
					"Type": {
						"IsRec": true
					},
					"Belt": {
						"IsRec": true
					},
					"Call": {
						"IsRec": true
					},
					"Crash": {
						"IsRec": true
					},
					"Danger": {
						"IsRec": true
					},
					"Rack": {
						"IsRec": true
					},
					"Sunroof": {
						"IsRec": true
					},
					"SpareTire": {
						"IsRec": true
					},
					"Slag": {
						"IsRec": true
					},
					"Convertible": {
						"IsRec": true
					},
					"Manned": {
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
}