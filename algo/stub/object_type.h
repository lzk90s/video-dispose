#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include <string>

namespace video {
namespace algo {

enum ObjectType {
    INVALID = -1,
    UNKNOWN = 0,		//未知
    PEDESTRAIN = 1,		//行人
    BIKE = 2,			//自行车
    MOTOBIKE = 3,		//摩托车
    CAR = 4,			//小汽车
    TRICYCLE = 5,		//三轮车
    BUS = 6,			//公交车
    VAN = 7,			//面包车
    TRUCK = 8,			//卡车
    FACE = 100,			//人脸
};


//目标属性
class Attribute {
public:
    std::string name;	// 属性值
    bool visable;	// 是否在流中可见（流叠加）
    uint32_t score;	// 评分

    Attribute() {
        visable = true;
        score = 0;
    }

    Attribute(const std::string &name, uint32_t score) {
        this->name = name;
        this->score = score;
    }

    Attribute &WithName(const std::string &name) {
        this->name = name;
        return *this;
    }

    Attribute &WithScore(uint32_t score) {
        this->score = score;
        return *this;
    }
};

typedef std::vector<int32_t> Rect;		//矩形[x,y,w,h]
typedef std::vector<int32_t> Shift;		//位移[x,y]
typedef std::vector<int32_t> Point;		//点[x,y]
typedef std::map<uint32_t, Attribute> Attributes;


// 目标基础类型
class BaseObject {
public:
    std::string guid;				// 目标guid
    ObjectType type;			// 目标类型
    Rect detect;				// 目标所在区域
    Shift trail;				// 目标跟踪变化
    uint32_t score;				// 目标评分
    Attributes attrs; // 目标属性

    BaseObject() {
        type = UNKNOWN;
        score = 0;
    }
};


////////////////////////////////////////////////////////////////////////////////////////////
// 机动车
class VehicleObject : public BaseObject {
public:
    enum AttrType {
        COLOR,
        TYPE,
        BRAND,
        PLATE,
    };
};

// 机动车择优
class VehicleFilter : public BaseObject {
public:
    std::string contextCode;			// 上下文code
    int32_t frameId;		// 最优帧的图像id

    VehicleFilter() {
        frameId = 0;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////
// 人
class PersonObject : public BaseObject {
public:
    enum AttrType {
        SEX,
        AGE,
        UPPER_COLOR,
        BOTTOM_COLOR,
        ORIENTATION,
        HAIR,
        UMBERLLA,
        HAT,
        UPPER_TYPE,
        BOTTOM_TYPE,
        KNAPSACK,
        BAG,
        BABY,
        MESSAGER_BAG,
        SHOULDER_BAG,
        GLASSES,
        MASK,
        UPPER_TEXTURE,
        BARROW,
        TROLLEY_CASE
    };
};

// 人择优
class PersonFilter : public BaseObject {
public:
    std::string contextCode;			// 上下文code
    int32_t frameId;		// 最优帧的图像id

    PersonFilter() {
        frameId = 0;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////
// 非机动车
class BikeObject : public BaseObject {
public:
    std::vector<PersonObject> persons;		// 自行车上的人
};

// 非机动车择优
class BikeFilter : public BaseObject {
public:
    std::string contextCode;			// 上下文code
    int32_t frameId;		// 最优帧的图像id

    BikeFilter() {
        frameId = 0;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////
// 人脸
class FaceObject : public BaseObject {
};

// 人脸择优
class FaceFilter : public BaseObject {
public:
    std::string contextCode;			// 上下文code
    int32_t frameId;		// 最优帧的图像id

    FaceFilter() {
        frameId = 0;
    }
};

}
}