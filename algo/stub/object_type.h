#pragma once

#include <memory>
#include <cstdint>
#include <vector>
#include <map>
#include <string>
#include <string.h>

using namespace std;

namespace algo {

//λ��
class Shift {
public:
    int32_t sx, sy;	//λ��X,Y
};

//��������
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

//Ŀ������
class Attribute {
public:
    string name;	// ����ֵ
    bool visable;	//�Ƿ������пɼ��������ӣ�

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
    UNKNOWN = 0,		//δ֪
    PEDESTRAIN = 1,		//����
    BIKE = 2,			//���г�
    MOTOBIKE = 3,		//Ħ�г�
    CAR = 4,			//С����
    TRICYCLE = 5,		//���ֳ�
    BUS = 6,			//������
    VAN = 7,			//�����
    TRUCK = 8,			//����
    FACE = 100,			//����
};

// Ŀ���������
class BaseObject {
public:
    string guid;				// Ŀ��guid
    ObjectType type;			// Ŀ������
    Rect detect;				// Ŀ����������
    vector<int32_t> trail;		// Ŀ����ٱ仯

    BaseObject() {
        memset(this, 0, sizeof(BaseObject));
    }
};

////////////////////////////////////////////////////////////////////////////////////////////
// ������
class VehicleObject : public BaseObject {
public:
    struct {
        Attribute color;
        Attribute type;
        Attribute brand;
        Attribute plate;
    } attrs;
};

// ����������
class VehicleFilter : public BaseObject {
public:
    string contextCode;			// ������code
    int32_t frameId;		// ����֡��ͼ��id
};

////////////////////////////////////////////////////////////////////////////////////////////
// ��
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

// ������
class PersonFilter : public BaseObject {
public:
    string contextCode;			// ������code
    int32_t frameId;		// ����֡��ͼ��id
};

////////////////////////////////////////////////////////////////////////////////////////////
// �ǻ�����
class BikeObject : public BaseObject {
public:
    vector<PersonObject> persons;		// ���г��ϵ���
};

// �ǻ���������
class BikeFilter : public BaseObject {
public:
    string contextCode;			// ������code
    int32_t frameId;		// ����֡��ͼ��id
};

////////////////////////////////////////////////////////////////////////////////////////////
// ����
class FaceObject : public BaseObject {
};

// ��������
class FaceFilter : public BaseObject {
public:
    string contextCode;			// ������code
    int32_t frameId;		// ����֡��ͼ��id
};

}
