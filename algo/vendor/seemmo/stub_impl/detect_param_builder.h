#pragma  once

#include <string>
#include "json/json.hpp"

namespace video {
namespace algo {
namespace seemmo {
namespace detect {

using json = nlohmann::json;

typedef struct tagDetectRegionPO {
    // [[x1,y1], [x2,y2], [x3,y3]....]
    std::vector<std::vector<int32_t>> regions;
} DetectRegionPO;

class DetectParamBuilder {
public:
    std::string Build(DetectRegionPO &r) {
        json j = R"(
        {
        "Detect": {
			"IsDet": true
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
}