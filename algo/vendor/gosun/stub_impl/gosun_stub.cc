
#include "common/helper/logger.h"
#include "algo/vendor/gosun/stub_impl/gosun_stub.h"

#include "interface/faceApi.h"

namespace algo {
namespace gosun {

GosunAlgoStub::GosunAlgoStub() {
    char *gosunSdkHome = getenv("GOSUN_SDK_HOME");
    if (nullptr != gosunSdkHome) {
        string h(gosunSdkHome);
        h.append("/lib");
        chdir(h.c_str());
    }
    //createSmartFace();
}

GosunAlgoStub::~GosunAlgoStub() {
    //releaseSmartFace();
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

    return 0;

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