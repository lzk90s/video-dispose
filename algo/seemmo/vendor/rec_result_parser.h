#pragma once

#include <vector>
#include <cstdint>
#include <map>

#include "common/helper/logger.h"

#include "json/json.hpp"


using namespace  std;
using json = nlohmann::json;

namespace rec {

typedef struct tagAttribute {
    string Code;
    string Name;
    int32_t Score;
} Attribute;

typedef struct tagAttributeItem {
    int32_t Code;
    string Message;
    vector<Attribute> TopList;
} AttributeItem;

typedef struct tagVehicleAttributeGroup {
    AttributeItem Color;
    AttributeItem Type;
    AttributeItem Brand;
    AttributeItem Plate;
} VehicleAttributeGroup;

typedef struct tagPersonAttributeGroup {
    AttributeItem Sex;
    AttributeItem Age;
    AttributeItem UpperColor;
    AttributeItem BottomColor;
    AttributeItem Orientation;
    AttributeItem Hair;
    AttributeItem Umbrella;
    AttributeItem Hat;
    AttributeItem UpperType;
    AttributeItem BottomType;
    AttributeItem Knapsack;
    AttributeItem Bag;
    AttributeItem Baby;
    AttributeItem MessengerBag;
    AttributeItem ShoulderBag;
    AttributeItem Glasses;
    AttributeItem Mask;
    AttributeItem UpperTexture;
    AttributeItem Barrow;
    AttributeItem TrolleyCase;
} PersonAttributeGroup;

typedef struct tagDetectArea {
    int32_t Code;
    string Message;
    struct tagBody {
        vector<int32_t> Rect;
        //int32_t Score;
    } Body;

    struct tagCar {
        vector<int32_t> Rect;
    } Car;

    tagDetectArea() {
        memset(this, 0, sizeof(tagDetectArea));
    }
} DetectArea;


typedef struct tagRecItem {
    int32_t Type;
    DetectArea Detect;
    //map<string, AttributeItem> Recognize;

    //just for bikes
    vector<struct tagRecItem> Persons;

    tagRecItem() {
        memset(this, 0, sizeof(tagRecItem));
    }
} RecObject;


typedef struct tagImageResult {
    int32_t Code;
    string Message;
    vector<RecObject> Vehicles;
    vector<RecObject> Pedestrains;
    vector<RecObject> Bikes;
} ImageResult;

typedef struct tagRecResponseMsg {
    int32_t Code;
    string Message;
    vector<ImageResult> ImageResults;
} RecResponseMsg;

void from_json(const json& j, Attribute& p) {
    try {
        p.Code = j.at("Code").get<string>();
        p.Name = j.at("Name").get<string>();
        p.Score = j.at("Score").get<int32_t>();
    } catch (exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

void from_json(const json& j, AttributeItem& p) {
    try {
        p.Code = j.at("Code").get<int32_t>();
        p.Message = j.at("Message").get<string>();
        if (0 == p.Code) {
            p.TopList = j.at("TopList").get<vector<Attribute>>();
        }
    } catch (exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

void from_json(const json& j, DetectArea& p) {
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

void from_json(const json &j, RecObject &p) {
    try {
        p.Type = j.at("Type").get<int>();
        p.Detect = j.at("Detect").get<DetectArea>();
        //p.Recognize = j.at("Recognize").get<map<string,AttributeItem>>();
        // add persons
        if (j.find("Persons") != j.end()) {
            p.Persons = j.at("Persons").get<vector<RecObject>>();
        }
    } catch (exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

void from_json(const json &j, ImageResult &p) {
    try {
        p.Code = j.at("Code").get<int32_t>();
        p.Message = j.at("Message").get<string>();
        if (0 == p.Code) {
            p.Vehicles = j.at("Vehicles").get<vector<RecObject>>();
            p.Bikes = j.at("Bikes").get<vector<RecObject>>();
            p.Pedestrains = j.at("Pedestrains").get<vector<RecObject>>();
        }
    } catch (exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

void from_json(const json &j, RecResponseMsg &p) {
    try {
        p.Code = j.at("Code").get<int32_t>();
        p.Message = j.at("Message").get<string>();
        if (0 == p.Code) {
            p.ImageResults = j.at("ImageResults").get<vector<ImageResult>>();
        }
    } catch (exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}


class RecResultParser {
public:
    void Parse(const string &response, RecResponseMsg &obj) {
        auto j = json::parse(response.c_str());
        obj = j.get<RecResponseMsg>();
    }

private:

};
}

