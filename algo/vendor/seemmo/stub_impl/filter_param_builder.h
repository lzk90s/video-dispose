#pragma  once

#include <string>
#include "json/json.hpp"

using namespace  std;
using json = nlohmann::json;

namespace algo {
namespace seemmo {
namespace trail {

typedef struct tagDetectRegionPO {
    // [[x1,y1], [x2,y2], [x3,y3]....]
    vector<vector<int32_t>> regions;
} DetectRegionPO;

class FilterParamBuilder {
public:
    string Build(DetectRegionPO &r) {
        json j = R"(
        {
        "Detect": {
            "IsDet": true,
            "Mode": 0,
            "Threshold": 20,
            "Version": 1002
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
