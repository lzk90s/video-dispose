#pragma  once

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "algo/stub/object_type.h"

using namespace  std;

namespace algo {

//检测跟踪参数
class TrailParam {
public:
};

//识别参数
class RecogParam {
public:
    string ContextCode;
    BaseObject obj;		//需要识别的目标
};

//检测结果
class DetectResult {
public:
    vector<VehicleObject> vehicles;		// 机动车
    vector<PersonObject> pedestrains;		// 行人
    vector<BikeObject> bikes;			// 非机动车
    vector<FaceObject> faces;			// 人脸
};

//跟踪择优结果
class FilterResult {
public:
    vector<VehicleFilter> vehicles;		// 机动车
    vector<PersonFilter> pedestrains;		// 行人
    vector<BikeFilter> bikes;			// 非机动车
    vector<FaceFilter> faces;			// 人脸
};

//识别结果
class RecogResult {
public:
    vector<VehicleObject> vehicles;		// 机动车
    vector<PersonObject> pedestrains;		// 行人
    vector<BikeObject> bikes;			// 非机动车
    vector<FaceObject> faces;			// 人脸
};

class AlgoStub {
public:
    AlgoStub(const string &vendor)  {
        vendor_ = vendor;
    }

    //检测跟踪 + 去重择优
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

    //识别
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