#pragma once

#include <stdint.h>

namespace algo {
namespace seemmo {

class AlgoStub {
public:
    AlgoStub();

    ~AlgoStub();

    int32_t VideoTrailAndRec(int32_t videoChl, uint64_t timeStamp, const uint8_t *bgr24, uint32_t height, uint32_t width);
};
}
}
