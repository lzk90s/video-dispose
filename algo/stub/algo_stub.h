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
    vector<Point> roi;	//ROI区域，每一个point是ROI多边形区域的一个点
};

//识别参数
class RecogParam {
public:
    typedef struct tagObjLocation {
        string ContextCode;
        ObjectType type;			// 目标类型
        Rect detect;		// 目标所在区域(x,y,w,h)
        Shift trail;		// 目标跟踪变化(x,y)
    } ObjLocation;

    vector<ObjLocation> locations;
};

//检测结果
class ImageResult {
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
    vector<int32_t> releasedFrames;	// 可以释放的帧
};

class AlgoStub {
public:
    AlgoStub(const string &vendor) {
        vendor_ = vendor;
    }

    //检测跟踪 + 去重择优
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

    //识别
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