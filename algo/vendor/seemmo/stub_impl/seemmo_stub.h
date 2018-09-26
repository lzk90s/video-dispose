#pragma once

#include <memory>
#include <string>
#include <vector>

#include "algo/stub/algo_stub.h"

using namespace  std;

namespace algo {
namespace seemmo {

class SeemmoAlgoStub : public algo::AlgoStub {
public:
    SeemmoAlgoStub();

    ~SeemmoAlgoStub();

    int32_t Trail(
        uint32_t channelId,
        uint64_t frameId,
        uint8_t *bgr24,
        uint32_t width,
        uint32_t height,
        const TrailParam &param,
        ImageResult &image,
        FilterResult &filter
    ) override;

    int32_t Recognize(
        uint32_t channelId,
        uint8_t *bgr24,
        uint32_t width,
        uint32_t height,
        const RecogParam &param,
        ImageResult &rec
    ) override;
};

}
}
