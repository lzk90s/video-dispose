#pragma once

#include "algo/stub/algo_stub.h"

namespace video {
namespace algo {
namespace seemmo {

class AlgoStubImpl : public algo::AlgoStub {
public:
    AlgoStubImpl() {};

    ~AlgoStubImpl() {};

    int32_t Trail(
        uint32_t channelId,
        uint64_t frameId,
        const uint8_t *bgr24,
        uint32_t width,
        uint32_t height,
        const TrailParam &param,
        ImageResult & imageResult,
        FilterResult & filterResult
    ) override;

    int32_t TrailEnd(uint32_t channelId) override;

    int32_t Recognize(
        uint32_t channelId,
        const uint8_t *bgr24,
        uint32_t width,
        uint32_t height,
        const RecogParam &param,
        ImageResult & imageResult
    ) override;
};

}
}
}