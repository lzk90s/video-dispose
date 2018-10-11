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

    Attribute(const string &name, uint32_t score) {
        this->name = name;
        this->score = score;
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

typedef vector<int32_t> Rect;		//����[x,y,w,h]
typedef vector<int32_t> Shift;		//λ��[x,y]
typedef vector<int32_t> Point;		//��[x,y]
typedef map<uint32_t, Attribute> Attributes;


// Ŀ���������
class BaseObject {
public:
    string guid;				// Ŀ��guid
    ObjectType type;			// Ŀ������
    Rect detect;				// Ŀ����������
    Shift trail;				// Ŀ����ٱ仯
    uint32_t score;				// Ŀ������
    Attributes attrs; // Ŀ������

    BaseObject() {
        type = UNKNOWN;
        score = 0;
    }
};


////////////////////////////////////////////////////////////////////////////////////////////
// ������
class VehicleObject : public BaseObject {
public:
    enum AttrType {
        COLOR,
        TYPE,
        BRAND,
        PLATE,
    } ;
};

// ����������
class VehicleFilter : public BaseObject {
public:
    string contextCode;			// ������code
    int32_t frameId;		// ����֡��ͼ��id

    VehicleFilter() {
        frameId = 0;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////
// ��
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

// ������
class PersonFilter : public BaseObject {
public:
    string contextCode;			// ������code
    int32_t frameId;		// ����֡��ͼ��id

    PersonFilter() {
        frameId = 0;
    }
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

    BikeFilter() {
        frameId = 0;
    }
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

    FaceFilter() {
        frameId = 0;
    }
};

}
