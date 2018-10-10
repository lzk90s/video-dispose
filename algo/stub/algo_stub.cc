
#include "common/helper/logger.h"
#include "common/helper/threadpool.h"
#include "algo/stub/algo_stub.h"
#include "algo/vendor/seemmo/stub_impl/seemmo_stub.h"
#include "algo/vendor/gosun/stub_impl/gosun_stub.h"

namespace algo {

class AlgoStubDelegate : public AlgoStub {
public:

    AlgoStubDelegate(bool enableSeemmoAlgo = true, bool enableGosunAlgo = true)
        : tp_(2) {
        enableSeemmoAlgo_ = enableSeemmoAlgo;
        enableGosunAlgo_ = enableGosunAlgo;

        if (enableSeemmoAlgo) {
            seemmoAlgo_.reset(new algo::seemmo::SeemmoAlgoStub());
        }
        if (enableGosunAlgo) {
            gosunAlgo_.reset(new algo::gosun::GosunAlgoStub());
        }
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
        const TrailParam *paramPtr = &param;
        ImageResult *imageResultPtr = &imageResult;
        FilterResult *filterResultPtr = &filterResult;

        //lamba捕获引用会出现莫名其妙的崩溃，所以，这里改为捕获值
        auto f1 = tp_.commit([=]() {
            if (enableSeemmoAlgo_) {
                return seemmoAlgo_->Trail(channelId, frameId, bgr24, width, height, *paramPtr, *imageResultPtr, *filterResultPtr);
            } else {
                return 0;
            }
        });
        auto f2 = tp_.commit([=]() {
            if (enableGosunAlgo_) {
                return gosunAlgo_->Trail(channelId, frameId, bgr24, width, height, *paramPtr, *imageResultPtr, *filterResultPtr);
            } else {
                return 0;
            }
        });

        int ret1 = f1.get();
        if (0 != ret1) {
            LOG_ERROR("SeemmoTrail error, ret {}", ret1);
        }
        int ret2 = f2.get();
        if (0 != ret2) {
            LOG_ERROR("GosunTrail error, ret {}", ret1);
        }

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
        const RecogParam *paramPtr = &param;
        ImageResult *imageResultPtr = &imageResult;

        //lamba捕获引用会出现莫名其妙的崩溃，所以，这里改为捕获值
        auto f1 = tp_.commit([=]() {
            if (enableSeemmoAlgo_) {
                return seemmoAlgo_->Recognize(channelId, bgr24, width, height, *paramPtr, *imageResultPtr);
            } else {
                return 0;
            }
        });
        auto f2 = tp_.commit([=]() {
            if (enableGosunAlgo_) {
                return gosunAlgo_->Recognize(channelId, bgr24, width, height, *paramPtr, *imageResultPtr);
            } else {
                return 0;
            }
        });

        int ret1 = f1.get();
        if (0 != ret1) {
            LOG_ERROR("SeemmoRecognize error, ret {}", ret1);
        }
        int ret2 = f2.get();
        if (0 != ret2) {
            LOG_ERROR("GosunRecognize error, ret {}", ret1);
        }

        return ret1 * ret2;	//只有2个都失败才认为失败，所以直接用*
    };

private:
    unique_ptr<algo::seemmo::SeemmoAlgoStub> seemmoAlgo_;
    unique_ptr<algo::gosun::GosunAlgoStub> gosunAlgo_;
    threadpool tp_;
    bool enableSeemmoAlgo_;
    bool enableGosunAlgo_;
};


AlgoStub *NewAlgoStub(bool enableSeemmoAlgo, bool enableGosunAlgo) {
    return new AlgoStubDelegate(enableSeemmoAlgo, enableGosunAlgo);
}

void FreeAlgoStub(AlgoStub *&stub) {
    delete stub;
    stub = nullptr;
}
}