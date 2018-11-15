#pragma once

#include <memory>
#include <string>
#include <vector>

#include "algo/stub/algo_stub.h"

namespace algo {
namespace gosun {

class GosunAlgoStub : public algo::AlgoStub {
public:
    GosunAlgoStub();

    ~GosunAlgoStub();

    int32_t Trail(
        uint32_t channelId,
        uint64_t frameId,
        const uint8_t *bgr24,
        uint32_t width,
        uint32_t height,
        const TrailParam &param,
        ImageResult &image,
        FilterResult &filter
    ) override;

    int32_t Recognize(
        uint32_t channelId,
        const uint8_t *bgr24,
        uint32_t width,
        uint32_t height,
        const RecogParam &param,
        ImageResult &rec
    ) override;

private:
    bool startOk_;
};

}
}