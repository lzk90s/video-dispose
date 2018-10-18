#include <memory>

#include "common/helper/logger.h"
#include "algo/vendor/gosun/stub_impl/gosun_stub.h"

#include "interface/faceApi.h"

using namespace std;

namespace algo {
namespace gosun {

GosunAlgoStub::GosunAlgoStub() {
    char *gosunSdkHome = getenv("GOSUN_SDK_HOME");
    if (nullptr != gosunSdkHome) {
        string h(gosunSdkHome);
        h.append("/lib");
        if (chdir(h.c_str()) != 0) {
            LOG_ERROR("chdir error");
        }
    }

    createSmartFace();

    //第一次算法比较慢，用空数据做几次算法热身
    uint32_t count = 5;
    do {
        uint32_t w = 1920;
        uint32_t h = 1080;
        faceId faceIds[30] = { 0 };
        int32_t faceNum = 30;
        unique_ptr<uint8_t[]> emptyImg(new uint8_t[w*h * 3] {0});

        int ret = faceTrack((uint8_t*)emptyImg.get(), h, w, faceIds, faceNum);
        if (0 != ret) {
            LOG_ERROR("Failed to track face, ret {}", ret);
            continue;
        }
        count--;
    } while (count >0);

    LOG_INFO("Gosun algo init ok");
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
    return 0;
}

}
}