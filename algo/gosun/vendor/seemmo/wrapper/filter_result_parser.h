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

typedef struct tagDetectArea {
    int32_t Code;
    string Message;
    struct tagBody {
        vector<int32_t> Rect;
        vector<int32_t> MarginRect;
        int32_t Score;
    } Body;

    tagDetectArea() {
        memset(this, 0, sizeof(tagDetectArea));
    }
} DetectArea;

typedef struct tagTrailItem {
    int32_t Type;
    string ContextCode;
    int64_t Timestamp;
    string GUID;
    vector<int32_t> Trail;
    DetectArea Detect;

    tagTrailItem() {
        memset(this, 0, sizeof(tagTrailItem));
    }
} TrailItem;


typedef struct tagFilterResult {
    int32_t Code;
    string Message;
    vector<TrailItem> Vehicles;
    vector<TrailItem> Pedestrains;
    vector<TrailItem> Bikes;

    tagFilterResult() {
        memset(this, 0, sizeof(tagFilterResult));
    }
} FilterResult;

typedef struct tagTrailResponseMsg {
    int32_t Code;
    string Message;
    vector<FilterResult> FilterResults;

    tagTrailResponseMsg() {
        memset(this, 0, sizeof(tagTrailResponseMsg));
    }
} TrailResponseMsg;


void from_json(const json& j, DetectArea& p) {
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

void from_json(const json &j, TrailItem &p) {
    try {
        p.Type = j.at("Type").get<int>();
        p.ContextCode = j.at("ContextCode").get<string>();
        p.Timestamp = j.at("Timestamp").get<int>();
        p.GUID = j.at("GUID").get<string>();
        p.Trail = j.at("Trail").get<vector<int>>();
        p.Detect = j.at("Detect").get<DetectArea>();
    } catch (exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

void from_json(const json &j, FilterResult &p) {
    try {
        p.Code = j.at("Code").get<int32_t>();
        p.Message = j.at("Message").get<string>();
        if (0 == p.Code) {
            p.Vehicles = j.at("Vehicles").get<vector<TrailItem>>();
            p.Bikes = j.at("Bikes").get<vector<TrailItem>>();
            p.Pedestrains = j.at("Pedestrains").get<vector<TrailItem>>();
        }
    } catch (exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

void from_json(const json &j, TrailResponseMsg &p) {
    try {
        p.Code = j.at("Code").get<int32_t>();
        p.Message = j.at("Message").get<string>();
        if (0 == p.Code) {
            p.FilterResults = j.at("FilterResults").get<vector<FilterResult>>();
        }
    } catch (exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

class FilterResponseParser {
public:
    void Parse(const string &response, TrailResponseMsg &obj) {
        auto j = json::parse(response.c_str());
        obj = j.get<TrailResponseMsg>();
    }
private:

};
}
}
}