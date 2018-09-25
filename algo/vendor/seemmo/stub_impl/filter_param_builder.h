#pragma  once

#include <string>
#include "json/json.hpp"

using namespace  std;
using json = nlohmann::json;

namespace algo {
namespace seemmo {
namespace trail {

class FilterParamBuilder {
public:
    string Build() {
        json j = R"(
        {
        "Detect": {
            "DetectRegion": [
				[0,0],
				[1920,0],
				[1920,1080],
				[0,1080]
            ],
            "IsDet": true,
            "MaxCarWidth": 0,
            "MinCarWidth": 0,
            "Mode": 0,
            "Threshold": 20,
            "Version": 1001
        },
        "Recognize": {
            "Person": {
                "Age": {
                    "IsRec": true
                },
                "Bag": {
                    "IsRec": true
                },
                "BottomColor": {
                    "IsRec": true
                },
                "Hat": {
                    "IsRec": true
                },
                "Mistake": {
                    "IsRec": true
                },
                "Sex": {
                    "IsRec": true
                },
                "UpperColor": {
                    "IsRec": true
                },
                "Feature": {
                    "IsRec": false
                },
                "Knapsack": {
                    "IsRec": true
                }
            }
            }
        })"_json;
        return j.dump();
    }

private:
};
}
}
}
