#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "common/helper/logger.h"
#include "json/json.hpp"

namespace video {
namespace algo {
namespace seemmo {
namespace detect {

using json = nlohmann::json;

typedef struct tagDetectPO {
    int32_t Code;
    std::string Message;
    struct tagBody {
        std::vector<int32_t> Rect;
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
    }
} ObjectPO;


typedef struct tagImageResultPO {
    int32_t Code;
    std::string Message;
    std::vector<ObjectPO> Vehicles;
    std::vector<ObjectPO> Pedestrains;
    std::vector<ObjectPO> Bikes;

    tagImageResultPO() {
        Code = 0;
    }
} ImageResultPO;

typedef struct tagDetectReplyPO {
    int32_t Code;
    std::string Message;
    std::vector<ImageResultPO> ImageResults;

    tagDetectReplyPO() {
        Code = 0;
    }
} DetectReplyPO;


void from_json(const json& j, DetectPO& p) {
    try {
        p.Code = j.at("Code").get<int>();
        p.Message = j.at("Message").get<std::string>();
        if (0 == p.Code) {
            p.Body.Rect = j.at("Body").at("Rect").get<std::vector<int>>();
            p.Body.Score = j.at("Body").at("Score").get<int>();
        }
    } catch (std::exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

void from_json(const json &j, ObjectPO &p) {
    try {
        p.Type = j.at("Type").get<int>();
        p.GUID = j.at("GUID").get<std::string>();
        p.Trail = j.at("Trail").get<std::vector<int>>();
        p.Detect = j.at("Detect").get<DetectPO>();
    } catch (std::exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

void from_json(const json &j, ImageResultPO &p) {
    try {
        p.Code = j.at("Code").get<int32_t>();
        p.Message = j.at("Message").get<std::string>();
        if (0 == p.Code) {
            p.Vehicles = j.at("Vehicles").get<std::vector<ObjectPO>>();
            p.Bikes = j.at("Bikes").get<std::vector<ObjectPO>>();
            p.Pedestrains = j.at("Pedestrains").get<std::vector<ObjectPO>>();
        }
    } catch (std::exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

void from_json(const json &j, DetectReplyPO &p) {
    try {
        p.Code = j.at("Code").get<int32_t>();
        p.Message = j.at("Message").get<std::string>();
        if (0 == p.Code) {
            p.ImageResults = j.at("ImageResults").get<std::vector<ImageResultPO>>();
        }
    } catch (std::exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

class DetectResponseParser {
public:
    void Parse(const std::string &response, DetectReplyPO &obj) {
        auto j = json::parse(response.c_str());
        obj = j.get<DetectReplyPO>();
    }
private:

};
}
}
}
}