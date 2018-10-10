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
    vector<Point> roi;	//ROI����ÿһ��point��ROI����������һ����
};

//ʶ�����
class RecogParam {
public:
    typedef struct tagObjLocation {
        string ContextCode;
        ObjectType type;			// Ŀ������
        Rect detect;		// Ŀ����������(x,y,w,h)
        Shift trail;		// Ŀ����ٱ仯(x,y)
    } ObjLocation;

    vector<ObjLocation> locations;
};

//�����
class ImageResult {
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
    vector<int32_t> releasedFrames;	// �����ͷŵ�֡
};

class AlgoStub {
public:
    //������ + ȥ������
    virtual int32_t Trail(
        uint32_t channelId,
        uint64_t frameId,
        const uint8_t *bgr24,
        uint32_t width,
        uint32_t height,
        const TrailParam &param,
        ImageResult &imageResult,
        FilterResult &filterResult
    ) {
        throw runtime_error("unimplemented method");
    };

    //ʶ��
    virtual int32_t Recognize(
        uint32_t channelId,
        const uint8_t *bgr24,
        uint32_t width,
        uint32_t height,
        const RecogParam &param,
        ImageResult &imageResult
    ) {
        throw runtime_error("unimplemented method");
    };
};

AlgoStub *NewAlgoStub(bool enableSeemmoAlgo = true, bool enableGosunAlgo = true);

void FreeAlgoStub(AlgoStub *&stub);

}