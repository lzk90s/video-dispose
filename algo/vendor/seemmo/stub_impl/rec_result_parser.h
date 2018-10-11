#pragma once

#include <vector>
#include <cstdint>
#include <map>

#include "common/helper/logger.h"

#include "json/json.hpp"


using namespace  std;
using json = nlohmann::json;

namespace algo {
namespace seemmo {
namespace rec {

typedef struct tagAttributePO {
    string Code;
    string Name;
    int32_t Score;

    tagAttributePO() {
        Score = 0;
    }
} AttributePO;

typedef struct tagVehicleAttributeGroup {
    AttributePO Color;
    AttributePO Type;
    AttributePO Brand;
    AttributePO Plate;
} VehicleAttributeGroup;

typedef struct tagPersonAttributeGroup {
    AttributePO Sex;
    AttributePO Age;
    AttributePO UpperColor;
    AttributePO BottomColor;
    AttributePO Orientation;
    AttributePO Hair;
    AttributePO Umbrella;
    AttributePO Hat;
    AttributePO UpperType;
    AttributePO BottomType;
    AttributePO Knapsack;
    AttributePO Bag;
    AttributePO Baby;
    AttributePO MessengerBag;
    AttributePO ShoulderBag;
    AttributePO Glasses;
    AttributePO Mask;
    AttributePO UpperTexture;
    AttributePO Barrow;
    AttributePO TrolleyCase;
} PersonAttributeGroupPO;

typedef struct tagDetectRectPO {
    int32_t Code;
    string Message;
    struct tagBody {
        vector<int32_t> Rect;
        //int32_t Score;
    } Body;

    struct tagCar {
        vector<int32_t> Rect;
    } Car;

    tagDetectRectPO() {
        Code = 0;
    }
} DetectRectPO;

// 人
typedef struct tagPersonObjectPO {
    int32_t Type;
    string GUID;
    DetectRectPO Detect;
    PersonAttributeGroupPO Recognize;

    tagPersonObjectPO() {
        Type = 0;
    }
} PersonObjectPO;

// 非机动车
typedef struct tagBikeObjectPO {
    int32_t Type;
    string GUID;
    DetectRectPO Detect;
    vector<PersonObjectPO> Persons;	//非机动车上的人

    tagBikeObjectPO() {
        Type = 0;
    }
} BikeObjectPO;

// 机动车
typedef struct tagVehicleObjectPO {
    int32_t Type;
    string GUID;
    DetectRectPO Detect;
    VehicleAttributeGroup Recognize;

    tagVehicleObjectPO() {
        Type = 0;
    }
} VehicleObjectPO;


typedef struct tagImageResultPO {
    int32_t Code;
    string Message;
    vector<VehicleObjectPO> Vehicles;
    vector<PersonObjectPO> Pedestrains;
    vector<BikeObjectPO> Bikes;

    tagImageResultPO() {
        Code = 0;
    }
} ImageResultPO;

typedef struct tagRecogReplyPO {
    int32_t Code;
    string Message;
    vector<ImageResultPO> ImageResults;

    tagRecogReplyPO() {
        Code = 0;
    }
} RecogReplyPO;

void from_json(const json& j, AttributePO& p) {
    try {
        int32_t code = j.at("Code").get<int32_t>();
        if (0 == code) {
            json toplist = j.at("TopList");
            //只取一个
            for (auto it = toplist.begin(); it != toplist.end(); ++it) {
                p.Code = (*it).at("Code").get<string>();
                p.Name = (*it).at("Name").get<string>();
                p.Score = (*it).at("Score").get<int32_t>();
                break;
            }
        }
    } catch (exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

void from_json(const json& j, VehicleAttributeGroup& p) {
    try {
        p.Brand = j.at("Brand").get<AttributePO>();
        p.Color = j.at("Color").get<AttributePO>();
        p.Type = j.at("Type").get<AttributePO>();

        //车牌单独解
        if (0 == j.at("Plate").at("Code")) {
            string license = j.at("Plate").at("Licence").get<string>();
            p.Plate.Name = license;
        }
    } catch (exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

void from_json(const json& j, PersonAttributeGroupPO& p) {
    try {
        p.Sex = j.at("Sex").get<AttributePO>();
        p.Age = j.at("Age").get<AttributePO>();
        p.UpperColor = j.at("UpperColor").get<AttributePO>();
        p.BottomColor = j.at("BottomColor").get<AttributePO>();
        p.Orientation = j.at("Orientation").get<AttributePO>();
        p.Hair = j.at("Hair").get<AttributePO>();
        p.Umbrella = j.at("Umbrella").get<AttributePO>();
        p.Hat = j.at("Hat").get<AttributePO>();
        p.UpperType = j.at("UpperType").get<AttributePO>();
        p.BottomType = j.at("BottomType").get<AttributePO>();
        p.Knapsack = j.at("Knapsack").get<AttributePO>();
        p.Bag = j.at("Bag").get<AttributePO>();
        p.Baby = j.at("Baby").get<AttributePO>();
        p.MessengerBag = j.at("MessengerBag").get<AttributePO>();
        p.ShoulderBag = j.at("ShoulderBag").get<AttributePO>();
        p.Glasses = j.at("Glasses").get<AttributePO>();
        p.Mask = j.at("Mask").get<AttributePO>();
        p.UpperTexture = j.at("UpperTexture").get<AttributePO>();
        p.Barrow = j.at("Barrow").get<AttributePO>();
        p.TrolleyCase = j.at("TrolleyCase").get<AttributePO>();
    } catch (exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

void from_json(const json& j, DetectRectPO& p) {
    try {
        p.Code = j.at("Code").get<int>();
        p.Message = j.at("Message").get<std::string>();
        if (0 == p.Code) {
            if (j.end() != j.find("Body")) {
                p.Body.Rect = j.at("Body").at("Rect").get<vector<int>>();
            }
            if (j.end() != j.find("Car")) {
                p.Car.Rect = j.at("Car").at("Rect").get<vector<int>>();
            }
        }
    } catch (exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

void from_json(const json &j, PersonObjectPO &p) {
    try {
        p.Type = j.at("Type").get<int>();
        p.GUID = j.at("GUID").get<string>();
        p.Detect = j.at("Detect").get<DetectRectPO>();
        p.Recognize = j.at("Recognize").get<PersonAttributeGroupPO>();
    } catch (exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

void from_json(const json &j, VehicleObjectPO &p) {
    try {
        p.Type = j.at("Type").get<int>();
        p.GUID = j.at("GUID").get<string>();
        p.Detect = j.at("Detect").get<DetectRectPO>();
        p.Recognize = j.at("Recognize").get<VehicleAttributeGroup>();
    } catch (exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

void from_json(const json &j, BikeObjectPO &p) {
    try {
        p.Type = j.at("Type").get<int>();
        p.GUID = j.at("GUID").get<string>();
        p.Detect = j.at("Detect").get<DetectRectPO>();
        p.Persons = j.at("Persons").get<vector<PersonObjectPO>>();
    } catch (exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

void from_json(const json &j, ImageResultPO &p) {
    try {
        p.Code = j.at("Code").get<int32_t>();
        p.Message = j.at("Message").get<string>();
        if (0 == p.Code) {
            p.Vehicles = j.at("Vehicles").get<vector<VehicleObjectPO>>();
            p.Bikes = j.at("Bikes").get<vector<BikeObjectPO>>();
            p.Pedestrains = j.at("Pedestrains").get<vector<PersonObjectPO>>();
        }
    } catch (exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

void from_json(const json &j, RecogReplyPO &p) {
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


class RecResultParser {
public:
    void Parse(const string &response, RecogReplyPO &obj) {
        auto j = json::parse(response.c_str());
        obj = j.get<RecogReplyPO>();
    }

private:

};
}
}
}