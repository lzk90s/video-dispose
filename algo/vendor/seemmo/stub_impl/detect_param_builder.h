#pragma  once

#include <string>
#include "json/json.hpp"

using namespace  std;
using json = nlohmann::json;

namespace algo {
namespace seemmo {
namespace detect {

typedef struct tagDetectRegionPO {
    // [[x1,y1], [x2,y2], [x3,y3]....]
    vector<vector<int32_t>> regions;
} DetectRegionPO;

class DetectParamBuilder {
public:
    string Build(DetectRegionPO &r) {
        json j = R"(
        {
        "Detect": {
			"IsDet": true,
			"MaxCarWidth": 0,
			"MinCarWidth": 0,
			"Mode": 0,
			"Threshold": 40,
			"Version": 1001
            }
        })"_json;

        j["Detect"]["DetectRegion"] = r.regions;

        return j.dump();
    }

private:
};
}
}
}
