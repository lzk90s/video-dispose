#pragma once

#include <memory>
#include <cstdint>
#include <vector>
#include <map>
#include <string>
#include <string.h>

using namespace std;

namespace algo {

enum ObjectType {
    INVALID = -1,
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

typedef vector<int32_t> Rect;		//����[x,y,w,h]
typedef vector<int32_t> Shift;		//λ��[x,y]
typedef vector<int32_t> Point;		//��[x,y]


// Ŀ���������
class BaseObject {
public:
    string guid;				// Ŀ��guid
    ObjectType type;			// Ŀ������
    Rect detect;				// Ŀ����������
    Shift trail;				// Ŀ����ٱ仯
    uint32_t score;				// Ŀ������

    BaseObject() {
        type = UNKNOWN;
        score = 0;
    }
};

//Ŀ������
class Attribute {
public:
    string name;	// ����ֵ
    bool visable;	// �Ƿ������пɼ��������ӣ�
    uint32_t score;	// ����

    Attribute() {
        visable = true;
        score = 0;
    }

    Attribute(const Attribute &rhs) {
        this->name = rhs.name;
        this->visable = rhs.visable;
    }

    Attribute &WithName(const string &name) {
        this->name = name;
        return *this;
    }

    Attribute &WithScore(uint32_t score) {
        this->score = score;
        return *this;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////
// ������
class VehicleObject : public BaseObject {
public:
    struct tagAttributeGroup {
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
    struct tagAttributeGroup {
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
