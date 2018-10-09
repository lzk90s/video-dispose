
#include "common/helper/logger.h"
#include "common/helper/threadpool.h"
#include "algo/stub/algo_stub.h"
#include "algo/vendor/seemmo/stub_impl/seemmo_stub.h"
#include "algo/vendor/gosun/stub_impl/gosun_stub.h"

namespace algo {

class AlgoStubDelegate : public AlgoStub {
public:

    AlgoStubDelegate()
        : tp_(2),
          seemmoAlgo_(new algo::seemmo::SeemmoAlgoStub()),
          gosunAlgo_(new algo::gosun::GosunAlgoStub()) {
    }

    int32_t Trail(
        uint32_t channelId,
        uint64_t frameId,
        const uint8_t *bgr24,
        uint32_t width,
        uint32_t height,
        const TrailParam &param,
        ImageResult &imageResult,
        FilterResult &filterResult
    ) {
        auto f1 = tp_.commit([&]() {
            return seemmoAlgo_->Trail(channelId, frameId, bgr24, width, height, param, imageResult, filterResult);
        });
        auto f2 = tp_.commit([&]() {
            return gosunAlgo_->Trail(channelId, frameId, bgr24, width, height, param, imageResult, filterResult);
        });

        int ret1 = f1.get();
        int ret2 = f2.get();

        return ret1 * ret2;	//只有2个都失败才认为失败，所以直接用*
    };

    int32_t Recognize(
        uint32_t channelId,
        const uint8_t *bgr24,
        uint32_t width,
        uint32_t height,
        const RecogParam &param,
        ImageResult &imageResult
    ) {
        auto f1 = tp_.commit([&]() {
            return seemmoAlgo_->Recognize(channelId,  bgr24, width, height, param, imageResult);
        });
        auto f2 = tp_.commit([&]() {
            return gosunAlgo_->Recognize(channelId, bgr24, width, height, param, imageResult);
        });

        int ret1 = f1.get();
        int ret2 = f2.get();

        return ret1 * ret2;	//只有2个都失败才认为失败，所以直接用*
    };

private:
    unique_ptr<algo::seemmo::SeemmoAlgoStub> seemmoAlgo_;
    unique_ptr<algo::gosun::GosunAlgoStub> gosunAlgo_;
    threadpool tp_;
};


AlgoStub *NewAlgoStub() {
    return new AlgoStubDelegate();
}

void FreeAlgoStub(AlgoStub *&stub) {
    delete stub;
    stub = nullptr;
}
}