#pragma once

#include <vector>
#include <cstdint>
#include "common/helper/logger.h"
#include "json/json.hpp"

using namespace  std;
using json = nlohmann::json;

namespace video {
namespace algo {
namespace seemmo {
namespace trail {

typedef struct tagDetectPO {
    int32_t Code;
    std::string Message;
    struct tagBody {
        std::vector<int32_t> Rect;
        std::vector<int32_t> MarginRect;
        int32_t Score;
    } Body;

    tagDetectPO() {
        Code = 0;
    }
} DetectPO;

typedef struct tagObjectPO {
    int32_t Type;
    std::string ContextCode;
    int64_t Timestamp;
    std::string GUID;
    std::vector<int32_t> Trail;
    DetectPO Detect;

    tagObjectPO() {
        Type = 0;
        Timestamp = 0;
    }
} ObjectPO;


typedef struct tagFilterResultPO {
    int32_t Code;
    std::string Message;
    std::vector<ObjectPO> Vehicles;
    std::vector<ObjectPO> Pedestrains;
    std::vector<ObjectPO> Bikes;
    std::vector<int32_t> ReleaseCacheFrames;

    tagFilterResultPO() {
        Code = 0;
    }
} FilterResultPO;

typedef struct tagTrailReplyPO {
    int32_t Code;
    std::string Message;
    std::vector<FilterResultPO> FilterResults;

    tagTrailReplyPO() {
        Code = 0;
    }
} TrailReplyPO;


void from_json(const json& j, DetectPO& p) {
    try {
        p.Code = j.at("Code").get<int>();
        p.Message = j.at("Message").get<std::string>();
        if (0 == p.Code) {
            p.Body.Rect = j.at("Body").at("Rect").get<std::vector<int>>();
            p.Body.MarginRect = j.at("Body").at("MarginRect").get<std::vector<int>>();
            p.Body.Score = j.at("Body").at("Score").get<int>();
        }
    } catch (std::exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

void from_json(const json &j, ObjectPO &p) {
    try {
        p.Type = j.at("Type").get<int>();
        p.ContextCode = j.at("ContextCode").get<std::string>();
        p.Timestamp = j.at("Timestamp").get<int>();
        p.GUID = j.at("GUID").get<std::string>();
        p.Trail = j.at("Trail").get<std::vector<int>>();
        p.Detect = j.at("Detect").get<DetectPO>();
    } catch (std::exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

void from_json(const json &j, FilterResultPO &p) {
    try {
        p.Code = j.at("Code").get<int32_t>();
        p.Message = j.at("Message").get<std::string>();
        if (0 == p.Code) {
            p.Vehicles = j.at("Vehicles").get<std::vector<ObjectPO>>();
            p.Bikes = j.at("Bikes").get<std::vector<ObjectPO>>();
            p.Pedestrains = j.at("Pedestrains").get<std::vector<ObjectPO>>();
            p.ReleaseCacheFrames = j.at("ReleaseCacheFrames").get<std::vector<int32_t>>();
        }
    } catch (std::exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

void from_json(const json &j, TrailReplyPO &p) {
    try {
        p.Code = j.at("Code").get<int32_t>();
        p.Message = j.at("Message").get<std::string>();
        if (0 == p.Code) {
            p.FilterResults = j.at("FilterResults").get<std::vector<FilterResultPO>>();
        }
    } catch (std::exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

class FilterResponseParser {
public:
    void Parse(const std::string &response, TrailReplyPO &obj) {
        auto j = json::parse(response.c_str());
        obj = j.get<TrailReplyPO>();
    }
private:

};
}
}
}
}