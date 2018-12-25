#include <memory>
#include <thread>

#include "common/helper/logger.h"
#include "common/helper/counttimer.h"

#include "algo/vendor/gosun/stub_impl/algo_stub_impl.h"

#include "interface/faceApi.h"

namespace video {
namespace algo {
namespace gosun {

AlgoStubImpl::AlgoStubImpl() : latch(1) {
    startOk_ = false;

    //async start
    std::thread t([this]() {
        char *gosunSdkHome = getenv("GOSUN_SDK_HOME");
        if (nullptr != gosunSdkHome) {
            std::string h(gosunSdkHome);
            h.append("/lib");
            if (chdir(h.c_str()) != 0) {
                std::cout << "chdir error" << std::endl;
            }
        }
        createSmartFace();
        this->startOk_ = true;
        latch.countDown();
        std::cout << "Gosun algo init ok" << std::endl;
    });
    t.detach();
}

AlgoStubImpl::~AlgoStubImpl() {
    //必须等待启动完成后才能relese，否则可能会导致崩溃
    latch.wait();
    releaseSmartFace();
}

int32_t AlgoStubImpl::Trail(
    uint32_t channelId,
    uint64_t frameId,
    const uint8_t *bgr24,
    uint32_t width,
    uint32_t height,
    const TrailParam &param,
    ImageResult &imageResult,
    FilterResult &filterResult
) {
    if (!started()) {
        return -1;
    }

    CountTimer t1("gosun::AlgoStubImpl::Trail", 80 * 1000);

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

        std::string id = cur->id;
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

int32_t AlgoStubImpl::Recognize(
    uint32_t channelId,
    const uint8_t *bgr24,
    uint32_t width,
    uint32_t height,
    const RecogParam &param,
    ImageResult &imageResult
) {
    if (!started()) {
        return -1;
    }

    CountTimer t1("gosun::AlgoStubImpl::Recognize", 80 * 1000);

    return 0;
}

}
}
}