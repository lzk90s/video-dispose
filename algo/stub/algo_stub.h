#pragma  once

#include <string>
#include <vector>
#include <map>

#include "algo/stub/object_type.h"

namespace video {
namespace algo {

//检测跟踪参数
class TrailParam {
public:
    std::vector<Point> roi; //ROI区域，每一个point是ROI多边形区域的一个点
};

//识别参数
class RecogParam {
public:
    typedef struct tagObjLocation {
        std::string ContextCode;
        std::string guid;
        ObjectType type;            // 目标类型
        Rect detect;        // 目标所在区域(x,y,w,h)
        Shift trail;        // 目标跟踪变化(x,y)
    } ObjLocation;

    std::vector<ObjLocation> locations;
};

//检测结果
class ImageResult {
public:
    std::vector<VehicleObject> vehicles;        // 机动车
    std::vector<PersonObject> pedestrains;      // 行人
    std::vector<BikeObject> bikes;          // 非机动车
    std::vector<FaceObject> faces;          // 人脸
};

//跟踪择优结果
class FilterResult {
public:
    std::vector<VehicleFilter> vehicles;        // 机动车
    std::vector<PersonFilter> pedestrains;      // 行人
    std::vector<BikeFilter> bikes;          // 非机动车
    std::vector<FaceFilter> faces;          // 人脸
    std::vector<int32_t> releasedFrames;    // 可以释放的帧
};

class AlgoStub {
public:
    virtual ~AlgoStub() {
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
        return 0;
    };

    virtual int32_t TrailEnd(
        uint32_t channelId
    ) {
        return 0;
    }

    //识别
    virtual int32_t Recognize(
        uint32_t channelId,
        const uint8_t *bgr24,
        uint32_t width,
        uint32_t height,
        const RecogParam &param,
        ImageResult &imageResult
    ) {
        return 0;
    };
};

}
}