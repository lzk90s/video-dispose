
#include "common/helper/logger.h"
#include "common/helper/threadpool.h"
#include "common/helper/counttimer.h"

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
        gosunAlgoStartOk = false;

        if (enableSeemmoAlgo) {
            seemmoAlgo_.reset(new algo::seemmo::SeemmoAlgoStub());
        }
        if (enableGosunAlgo) {
            //由于高创算法还需要加载算法库，比较慢。异步加载
            tp_.commit([this]() {
                gosunAlgo_.reset(new algo::gosun::GosunAlgoStub());
                gosunAlgoStartOk = true;	//设置启动ok
            });
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
                CountTimer t1("Seemmo_Trail", 80 * 1000);
                return seemmoAlgo_->Trail(channelId, frameId, bgr24, width, height, *paramPtr, *imageResultPtr, *filterResultPtr);
            } else {
                return 0;
            }
        });
        auto f2 = tp_.commit([=]() {
            if (enableGosunAlgo_) {
                CountTimer t1("Gosun_Trail", 80*1000);
                //double check多线程可能有问题，不过影响不大，可以忽略
                if (gosunAlgoStartOk) {
                    if (gosunAlgoStartOk) {
                        return gosunAlgo_->Trail(channelId, frameId, bgr24, width, height, *paramPtr, *imageResultPtr, *filterResultPtr);
                    }
                }
                return -1;	// return error
            } else {
                return 0;
            }
        });

#if 0
        auto s1 = f1.wait_for(std::chrono::milliseconds(100));
        if (std::future_status::ready == s1) {
            int ret1 = f1.get();
            if (0 != ret1) {
                LOG_ERROR("SeemmoTrail error, ret {}", ret1);
            }
        } else if (std::future_status::timeout == s1) {
            LOG_WARN("SeemmoTrail timeout");
        }

        auto s2 = f2.wait_for(std::chrono::milliseconds(100));
        if (std::future_status::ready == s2) {
            int ret2 = f2.get();
            if (0 != ret2) {
                LOG_ERROR("GosunTrail error, ret {}", ret2);
            }
        } else if (std::future_status::timeout == s1) {
            LOG_WARN("GosunTrail timeout");
        }
#else
        int ret1 = f1.get();
        if (0 != ret1) {
            LOG_ERROR("SeemmoTrail error, ret {}", ret1);
        }

        int ret2 = f2.get();
        if (0 != ret2) {
            LOG_ERROR("GosunTrail error, ret {}", ret2);
        }
#endif

        return 0;

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
                CountTimer t1("Seemmo_Recognize", 80 * 1000);
                return seemmoAlgo_->Recognize(channelId, bgr24, width, height, *paramPtr, *imageResultPtr);
            } else {
                return 0;
            }
        });
        auto f2 = tp_.commit([=]() {
            if (enableGosunAlgo_) {
                CountTimer t1("Gosun_Recognize", 80 * 1000);
                //double check多线程可能有问题，不过影响不大，可以忽略
                if (gosunAlgoStartOk) {
                    if (gosunAlgoStartOk) {
                        return gosunAlgo_->Recognize(channelId, bgr24, width, height, *paramPtr, *imageResultPtr);
                    }
                }
                return -1;	//return error
            } else {
                return 0;
            }
        });

#if 0
        auto s1 = f1.wait_for(std::chrono::milliseconds(100));
        if (std::future_status::ready == s1) {
            int ret1 = f1.get();
            if (0 != ret1) {
                LOG_ERROR("SeemmoRecognize error, ret {}", ret1);
            }
        } else if (std::future_status::timeout == s1) {
            LOG_WARN("SeemmoRecognize timeout");
        }

        auto s2 = f2.wait_for(std::chrono::milliseconds(100));
        if (std::future_status::ready == s2) {
            int ret2 = f2.get();
            if (0 != ret2) {
                LOG_ERROR("GosunRecognize error, ret {}", ret2);
            }
        } else if (std::future_status::timeout == s1) {
            LOG_WARN("GosunRecognize timeout");
        }
#else
        int ret1 = f1.get();
        if (0 != ret1) {
            LOG_ERROR("SeemmoRecognize error, ret {}", ret1);
        }

        int ret2 = f2.get();
        if (0 != ret2) {
            LOG_ERROR("GosunRecognize error, ret {}", ret2);
        }
#endif

        return 0;
    };

private:
    unique_ptr<algo::seemmo::SeemmoAlgoStub> seemmoAlgo_;
    unique_ptr<algo::gosun::GosunAlgoStub> gosunAlgo_;
    threadpool tp_;
    bool enableSeemmoAlgo_;
    bool enableGosunAlgo_;
    bool gosunAlgoStartOk;
};


AlgoStub *NewAlgoStub(bool enableSeemmoAlgo, bool enableGosunAlgo) {
    return new AlgoStubDelegate(enableSeemmoAlgo, enableGosunAlgo);
}

void FreeAlgoStub(AlgoStub *&stub) {
    delete stub;
    stub = nullptr;
}
}