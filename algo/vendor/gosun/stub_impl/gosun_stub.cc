#include <memory>
#include <functional>
#include <future>

#include "common/helper/logger.h"
#include "common/helper/counttimer.h"

#include "algo/vendor/gosun/stub_impl/gosun_stub.h"

#include "interface/faceApi.h"

using namespace std;

namespace algo {
namespace gosun {

GosunAlgoStub::GosunAlgoStub() {
    startOk_ = false;
    //async start
    std::future<void> f1 = std::async(std::launch::async, [=]() {
        char *gosunSdkHome = getenv("GOSUN_SDK_HOME");
        if (nullptr != gosunSdkHome) {
            string h(gosunSdkHome);
            h.append("/lib");
            if (chdir(h.c_str()) != 0) {
                LOG_ERROR("chdir error");
            }
        }

        createSmartFace();
        this->startOk_ = true;
        LOG_INFO("Gosun algo init ok");
    });
}

GosunAlgoStub::~GosunAlgoStub() {
    releaseSmartFace();
}

int32_t GosunAlgoStub::Trail(
    uint32_t channelId,
    uint64_t frameId,
    const uint8_t *bgr24,
    uint32_t width,
    uint32_t height,
    const TrailParam &param,
    ImageResult &imageResult,
    FilterResult &filterResult
) {
    //double check多线程可能有问题，不过影响不大，可以忽略
    if (!startOk_) {
        if (!startOk_) {
            return -1;
        }
    }

    CountTimer t1("GosunAlgoStub::Trail", 80 * 1000);

    faceId faceIds[30] = { 0 };
    int32_t faceNum = 30;

    int ret = faceTrack((uint8_t*)bgr24, height, width, faceIds, faceNum);
    if (0 != ret) {
        LOG_ERROR("Failed to track face, ret {}", ret);
        return ret;
    }

    // convert
    for (int i = 0; i < faceNum; i++) {
        faceId *cur = &faceIds[i];

        string id = cur->id;
        if (id.empty()) {
            continue;
        }

        algo::FaceObject o;
        o.detect.push_back(cur->startx);
        o.detect.push_back(cur->starty);
        o.detect.push_back(cur->width);
        o.detect.push_back(cur->height);
        o.guid = id;
        o.type = ObjectType::FACE;

        imageResult.faces.push_back(o);
    }

    return 0;
}

int32_t GosunAlgoStub::Recognize(
    uint32_t channelId,
    const uint8_t *bgr24,
    uint32_t width,
    uint32_t height,
    const RecogParam &param,
    ImageResult &imageResult
) {
    //double check多线程可能有问题，不过影响不大，可以忽略
    if (!startOk_) {
        if (!startOk_) {
            return -1;
        }
    }

    CountTimer t1("GosunAlgoStub::Recognize", 80 * 1000);

    return 0;
}

}
}