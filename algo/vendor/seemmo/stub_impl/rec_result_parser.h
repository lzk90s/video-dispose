#pragma once

#include <vector>
#include <cstdint>
#include <map>

#include "common/helper/logger.h"

#include "json/json.hpp"

namespace video {
namespace algo {
namespace seemmo {
namespace rec {

using json = nlohmann::json;

typedef struct tagAttributePO {
    std::string Code;
    std::string Name;
    int32_t Score;

    tagAttributePO() {
        Score = 0;
    }
} AttributePO;

typedef struct tagVehicleAttributeGroup {
    AttributePO Color;	//车辆颜色
    AttributePO Type;	//车辆类型
    AttributePO Brand;	//品牌型号
    AttributePO Plate;	//号牌号码
    AttributePO Marker;	//标识物
    AttributePO Slag;	//渣土车识别
    AttributePO Rack;	//行李架识别
    AttributePO Sunroof;	//天窗识别
    AttributePO SpareTire;	//备胎识别
    AttributePO Belt;	//安全带识别
    AttributePO Call;	//打电话识别
    AttributePO Crash;	//车辆撞损识别
    AttributePO Danger;	//危险品车辆识别
    AttributePO Tricycle; //三轮车识别项
    AttributePO Convertible;	//是否敞篷
    AttributePO Manned;	//是否载人
} VehicleAttributeGroup;

typedef struct tagPersonAttributeGroup {
    AttributePO Sex;	//性别
    AttributePO Age;	//人的年龄
    AttributePO UpperColor;	//上身衣服颜色
    AttributePO BottomColor;	//下身衣服颜色
    AttributePO Orientation;	//人的朝向（前，后，左，右）
    AttributePO Hair;	//头发类型
    AttributePO Umbrella;	//是否打伞
    AttributePO Hat;	//是否带帽子以及头盔
    AttributePO UpperType;	//上身衣服类型
    AttributePO BottomType;	//下身衣服类型
    AttributePO Knapsack;	//是否背包
    AttributePO Bag;	//是否有拎东西
    AttributePO Baby;	//是否抱小孩
    AttributePO MessengerBag;	//
    AttributePO ShoulderBag;	//
    AttributePO Glasses;	//眼镜
    AttributePO Mask;	//口罩
    AttributePO UpperTexture;	//上衣纹理
    AttributePO Barrow;	//
    AttributePO TrolleyCase;	//
} PersonAttributeGroupPO;

typedef struct tagDetectRectPO {
    int32_t Code;
    std::string Message;
    struct tagBody {
        std::vector<int32_t> Rect;
        //int32_t Score;
    } Body;

    tagDetectRectPO() {
        Code = 0;
    }
} DetectRectPO;

// 人
typedef struct tagPersonObjectPO {
    int32_t Type;
    std::string GUID;
    DetectRectPO Detect;
    PersonAttributeGroupPO Recognize;

    tagPersonObjectPO() {
        Type = 0;
    }
} PersonObjectPO;

// 非机动车
typedef struct tagBikeObjectPO {
    int32_t Type;
    std::string GUID;
    DetectRectPO Detect;
    std::vector<PersonObjectPO> Persons;	//非机动车上的人

    tagBikeObjectPO() {
        Type = 0;
    }
} BikeObjectPO;

// 机动车
typedef struct tagVehicleObjectPO {
    int32_t Type;
    std::string GUID;
    DetectRectPO Detect;
    VehicleAttributeGroup Recognize;

    tagVehicleObjectPO() {
        Type = 0;
    }
} VehicleObjectPO;


typedef struct tagImageResultPO {
    int32_t Code;
    std::string Message;
    std::vector<VehicleObjectPO> Vehicles;
    std::vector<PersonObjectPO> Pedestrains;
    std::vector<BikeObjectPO> Bikes;

    tagImageResultPO() {
        Code = 0;
    }
} ImageResultPO;

typedef struct tagRecogReplyPO {
    int32_t Code;
    std::string Message;
    std::vector<ImageResultPO> ImageResults;

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
                p.Code = (*it).at("Code").get<std::string>();
                p.Name = (*it).at("Name").get<std::string>();
                p.Score = (*it).at("Score").get<int32_t>();
                break;
            }
        }
    } catch (std::exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

void from_json(const json& j, VehicleAttributeGroup& p) {
    try {
        if (j.find("Brand") != j.end()) {
            p.Brand = j.at("Brand").get<AttributePO>();
        }
        if (j.find("Color") != j.end()) {
            p.Color = j.at("Color").get<AttributePO>();
        }
        if (j.find("Type") != j.end()) {
            p.Type = j.at("Type").get<AttributePO>();
        }
        if (j.find("Plate") != j.end()) {
            //车牌单独解
            if (0 == j.at("Plate").at("Code")) {
                std::string license = j.at("Plate").at("Licence").get<std::string>();
                p.Plate.Name = license;
            }
        }
    } catch (std::exception &e) {
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
    } catch (std::exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

void from_json(const json& j, DetectRectPO& p) {
    try {
        p.Code = j.at("Code").get<int>();
        p.Message = j.at("Message").get<std::string>();
        if (0 == p.Code) {
            if (j.end() != j.find("Body")) {
                p.Body.Rect = j.at("Body").at("Rect").get<std::vector<int>>();
            }
            //car时，用car.rect覆盖body.rect
            if (j.end() != j.find("Car")) {
                p.Body.Rect = j.at("Car").at("Rect").get<std::vector<int>>();
            }
        }
    } catch (std::exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

void from_json(const json &j, PersonObjectPO &p) {
    try {
        p.Type = j.at("Type").get<int>();
        p.GUID = j.at("GUID").get<std::string>();
        p.Detect = j.at("Detect").get<DetectRectPO>();
        p.Recognize = j.at("Recognize").get<PersonAttributeGroupPO>();
    } catch (std::exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

void from_json(const json &j, VehicleObjectPO &p) {
    try {
        p.Type = j.at("Type").get<int>();
        p.GUID = j.at("GUID").get<std::string>();
        p.Detect = j.at("Detect").get<DetectRectPO>();
        p.Recognize = j.at("Recognize").get<VehicleAttributeGroup>();
    } catch (std::exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

void from_json(const json &j, BikeObjectPO &p) {
    try {
        p.Type = j.at("Type").get<int>();
        p.GUID = j.at("GUID").get<std::string>();
        p.Detect = j.at("Detect").get<DetectRectPO>();
        p.Persons = j.at("Persons").get<std::vector<PersonObjectPO>>();
    } catch (std::exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

void from_json(const json &j, ImageResultPO &p) {
    try {
        p.Code = j.at("Code").get<int32_t>();
        p.Message = j.at("Message").get<std::string>();
        if (0 == p.Code) {
            p.Vehicles = j.at("Vehicles").get<std::vector<VehicleObjectPO>>();
            p.Bikes = j.at("Bikes").get<std::vector<BikeObjectPO>>();
            p.Pedestrains = j.at("Pedestrains").get<std::vector<PersonObjectPO>>();
        }
    } catch (std::exception &e) {
        LOG_ERROR("Parse json error, {}, {}", j.dump(), e.what());
    }
}

void from_json(const json &j, RecogReplyPO &p) {
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


class RecResultParser {
public:
    void Parse(const std::string &response, RecogReplyPO &obj) {
        auto j = json::parse(response.c_str());
        obj = j.get<RecogReplyPO>();
    }

private:

};
}
}
}
}