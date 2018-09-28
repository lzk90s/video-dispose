#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "common/helper/logger.h"
#include "json/json.hpp"

using namespace  std;
using json = nlohmann::json;

namespace algo {
namespace seemmo {
namespace detect {

typedef struct tagDetectPO {
    int32_t Code;
    string Message;
    struct tagBody {
        vector<int32_t> Rect;
        int32_t Score;
    } Body;
} DetectPO;

typedef struct tagObjectPO {
    int32_t Type;
    string ContextCode;
    int64_t Timestamp;
    string GUID;
    vector<int32_t> Trail;
    DetectPO Detect;
} ObjectPO;


typedef struct tagImageResultPO {
    int32_t Code;
    string Message;
    vector<ObjectPO> Vehicles;
    vector<ObjectPO> Pedestrains;
    vector<ObjectPO> Bikes;
} ImageResultPO;

typedef struct tagDetectReplyPO {
    int32_t Code;
    string Message;
    vector<ImageResultPO> ImageResults;
} DetectReplyPO;


void from_json(const json& j, DetectPO& p) {
    try {
        p.Code = j.at("Code").get<int>();
        p.Message = j.at("Message").get<std::string>();
        if (0 == p.Code) {
            p.Body.Rect = j.at("Body").at("Rect").get<vector<int>>();
            p.Body.Score = j.at("Body").at("Score").get<int>();
        }
    } catch (exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

void from_json(const json &j, ObjectPO &p) {
    try {
        p.Type = j.at("Type").get<int>();
        p.GUID = j.at("GUID").get<string>();
        p.Trail = j.at("Trail").get<vector<int>>();
        p.Detect = j.at("Detect").get<DetectPO>();
    } catch (exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

void from_json(const json &j, ImageResultPO &p) {
    try {
        p.Code = j.at("Code").get<int32_t>();
        p.Message = j.at("Message").get<string>();
        if (0 == p.Code) {
            p.Vehicles = j.at("Vehicles").get<vector<ObjectPO>>();
            p.Bikes = j.at("Bikes").get<vector<ObjectPO>>();
            p.Pedestrains = j.at("Pedestrains").get<vector<ObjectPO>>();
        }
    } catch (exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

void from_json(const json &j, DetectReplyPO &p) {
    try {
        p.Code = j.at("Code").get<int32_t>();
        p.Message = j.at("Message").get<string>();
        if (0 == p.Code) {
            p.ImageResults = j.at("ImageResults").get<vector<ImageResultPO>>();
        }
    } catch (exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

class DetectResponseParser {
public:
    void Parse(const string &response, DetectReplyPO &obj) {
        auto j = json::parse(response.c_str());
        obj = j.get<DetectReplyPO>();
    }
private:

};
}
}
}