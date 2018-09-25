#include "algo/stub/algo_stub.h"
#include "algo/vendor/seemmo/stub_impl/seemmo_stub.h"

namespace algo {

int32_t AlgoStub::Trail(
    uint32_t channelId,
    uint64_t frameId,
    uint8_t *bgr24,
    uint32_t width,
    uint32_t height,
    const TrailParam &param,
    DetectResult &detect,
    FilterResult &filter) {
    throw runtime_error("unimplemented method: AlgoStub::Trail");
}

int32_t AlgoStub::Recognize(
    uint8_t *bgr24,
    uint32_t width,
    uint32_t height,
    const RecogParam &param,
    RecogResult &rec) {
    throw runtime_error("unimplemented method: AlgoStub::Recognize");
}


AlgoStub* AlgoStubFactory::CreateStub(const string &vendor) {
    if (vendor == "seemmo") {
        return algo::seemmo::NewAlgoStub();
    } else if (vendor == "gosun") {
        return nullptr;
    } else {
        throw runtime_error("invalid vendor " + vendor);
    }
}

void AlgoStubFactory::FreeStub(AlgoStub *&stub) {
    if (stub->GetVendor() == "seemmo") {
        return algo::seemmo::FreeAlgoStub(stub);
    } else if (stub->GetVendor() == "gosun") {
        //
    } else {
        throw runtime_error("invalid vendor " + stub->GetVendor());
    }
}

}