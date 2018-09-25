#pragma  once

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "algo/stub/object_type.h"

using namespace  std;

namespace algo {

//�����ٲ���
class TrailParam {
public:
};

//ʶ�����
class RecogParam {
public:
    string ContextCode;
    BaseObject obj;		//��Ҫʶ���Ŀ��
};

//�����
class DetectResult {
public:
    vector<VehicleObject> vehicles;		// ������
    vector<PersonObject> pedestrains;		// ����
    vector<BikeObject> bikes;			// �ǻ�����
    vector<FaceObject> faces;			// ����
};

//�������Ž��
class FilterResult {
public:
    vector<VehicleFilter> vehicles;		// ������
    vector<PersonFilter> pedestrains;		// ����
    vector<BikeFilter> bikes;			// �ǻ�����
    vector<FaceFilter> faces;			// ����
};

//ʶ����
class RecogResult {
public:
    vector<VehicleObject> vehicles;		// ������
    vector<PersonObject> pedestrains;		// ����
    vector<BikeObject> bikes;			// �ǻ�����
    vector<FaceObject> faces;			// ����
};

class AlgoStub {
public:
    AlgoStub(const string &vendor)  {
        vendor_ = vendor;
    }

    //������ + ȥ������
    virtual int32_t Trail(
        uint32_t channelId,
        uint64_t frameId,
        uint8_t *bgr24,
        uint32_t width,
        uint32_t height,
        const TrailParam &param,
        DetectResult &detect,
        FilterResult &filter
    );

    //ʶ��
    virtual int32_t Recognize(
        uint8_t *bgr24,
        uint32_t width,
        uint32_t height,
        const RecogParam &param,
        RecogResult &rec
    );

    string GetVendor() {
        return this->vendor_;
    }

private:
    string vendor_;
};

class AlgoStubFactory {
public:
    static AlgoStub* CreateStub(const string &vendor);

    static void FreeStub(AlgoStub *&stub);
};

}