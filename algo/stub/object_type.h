#pragma once

#include <memory>
#include <cstdint>
#include <vector>
#include <map>
#include <string>
#include <string.h>

using namespace std;

namespace algo {

//位移
class Shift {
public:
    int32_t sx, sy;	//位移X,Y
};

//举行区域
class Rect {
public:
    int32_t x, y, w, h;		//x,y  w,h

    Rect(int32_t x, int32_t y, int32_t w, int32_t h) {
        this->x = x;
        this->y = y;
        this->w = w;
        this->h = h;
    }

    Rect(const Rect &rhs) {
        this->x = rhs.x;
        this->y = rhs.y;
        this->w = rhs.w;
        this->h = rhs.h;
    }

    Rect() {
        x = y = w = h = -1;
    }

    void Reset() {
        x = y = w = h = -1;
    }

    void Set(int32_t x, int32_t y, int32_t w, int32_t h) {
        this->x = x;
        this->y = y;
        this->w = w;
        this->h = h;
    }
};

//目标属性
class Attribute {
public:
    string name;	// 属性值
    bool visable;	//是否在流中可见（流叠加）

    Attribute() {
        visable = true;
    }

    Attribute(const Attribute &rhs) {
        this->name = rhs.name;
        this->visable = rhs.visable;
    }

    Attribute &WithName(const string &name) {
        this->name = name;
        return *this;
    }
};

enum ObjectType {
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

// 目标基础类型
class BaseObject {
public:
    string guid;				// 目标guid
    ObjectType type;			// 目标类型
    Rect detect;				// 目标所在区域
    vector<int32_t> trail;		// 目标跟踪变化

    BaseObject() {
        memset(this, 0, sizeof(BaseObject));
    }
};

////////////////////////////////////////////////////////////////////////////////////////////
// 机动车
class VehicleObject : public BaseObject {
public:
    struct {
        Attribute color;
        Attribute type;
        Attribute brand;
        Attribute plate;
    } attrs;
};

// 机动车择优
class VehicleFilter : public BaseObject {
public:
    string contextCode;			// 上下文code
    int32_t frameId;		// 最优帧的图像id
};

////////////////////////////////////////////////////////////////////////////////////////////
// 人
class PersonObject : public BaseObject {
public:
    struct {
        Attribute sex;
        Attribute age;
        Attribute upperColor;
        Attribute bottomColor;
        Attribute orientation;
        Attribute hair;
        Attribute umbrella;
        Attribute hat;
        Attribute upperType;
        Attribute bottomType;
        Attribute knapsack;
        Attribute bag;
        Attribute baby;
        Attribute messengerBag;
        Attribute shoulderBag;
        Attribute glasses;
        Attribute mask;
        Attribute upperTexture;
        Attribute barrow;
        Attribute trolleyCase;
    } attrs;
};

// 人择优
class PersonFilter : public BaseObject {
public:
    string contextCode;			// 上下文code
    int32_t frameId;		// 最优帧的图像id
};

////////////////////////////////////////////////////////////////////////////////////////////
// 非机动车
class BikeObject : public BaseObject {
public:
    vector<PersonObject> persons;		// 自行车上的人
};

// 非机动车择优
class BikeFilter : public BaseObject {
public:
    string contextCode;			// 上下文code
    int32_t frameId;		// 最优帧的图像id
};

////////////////////////////////////////////////////////////////////////////////////////////
// 人脸
class FaceObject : public BaseObject {
};

// 人脸择优
class FaceFilter : public BaseObject {
public:
    string contextCode;			// 上下文code
    int32_t frameId;		// 最优帧的图像id
};

}
