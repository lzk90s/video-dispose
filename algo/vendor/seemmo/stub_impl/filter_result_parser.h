#pragma once

#include <vector>
#include <cstdint>
#include "common/helper/logger.h"
#include "json/json.hpp"

using namespace  std;
using json = nlohmann::json;

namespace algo {
namespace seemmo {
namespace trail {

typedef struct tagDetectPO {
    int32_t Code;
    string Message;
    struct tagBody {
        vector<int32_t> Rect;
        vector<int32_t> MarginRect;
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


typedef struct tagFilterResultPO {
    int32_t Code;
    string Message;
    vector<ObjectPO> Vehicles;
    vector<ObjectPO> Pedestrains;
    vector<ObjectPO> Bikes;
} FilterResultPO;

typedef struct tagTrailReplyPO {
    int32_t Code;
    string Message;
    vector<FilterResultPO> FilterResults;
} TrailReplyPO;


void from_json(const json& j, DetectPO& p) {
    try {
        p.Code = j.at("Code").get<int>();
        p.Message = j.at("Message").get<std::string>();
        if (0 == p.Code) {
            p.Body.Rect = j.at("Body").at("Rect").get<vector<int>>();
            p.Body.MarginRect = j.at("Body").at("MarginRect").get<vector<int>>();
            p.Body.Score = j.at("Body").at("Score").get<int>();
        }
    } catch (exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

void from_json(const json &j, ObjectPO &p) {
    try {
        p.Type = j.at("Type").get<int>();
        p.ContextCode = j.at("ContextCode").get<string>();
        p.Timestamp = j.at("Timestamp").get<int>();
        p.GUID = j.at("GUID").get<string>();
        p.Trail = j.at("Trail").get<vector<int>>();
        p.Detect = j.at("Detect").get<DetectPO>();
    } catch (exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

void from_json(const json &j, FilterResultPO &p) {
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

void from_json(const json &j, TrailReplyPO &p) {
    try {
        p.Code = j.at("Code").get<int32_t>();
        p.Message = j.at("Message").get<string>();
        if (0 == p.Code) {
            p.FilterResults = j.at("FilterResults").get<vector<FilterResultPO>>();
        }
    } catch (exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

class FilterResponseParser {
public:
    void Parse(const string &response, TrailReplyPO &obj) {
        auto j = json::parse(response.c_str());
        obj = j.get<TrailReplyPO>();
    }
private:

};
}
}
}