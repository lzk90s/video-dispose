#include <iostream>
#include <string>
#include <memory>
#include <sstream>

#include "common/helper/threadpool.h"
#include "common/helper/countdownlatch.h"
#include "common/helper/counttimer.h"
#include "common/helper/logger.h"

#include "sdk_export/sdk_export.h"

#include "algo/vendor/seemmo/wrapper/algo_wrapper.h"

using namespace std;

namespace algo {
namespace seemmo {

class TrailWorker {
public:
    TrailWorker(uint32_t gpuDevId)
        : thrNum_(1),
          cw_(thrNum_),
          tp_(thrNum_, std::bind(&TrailWorker::threadInitProc, this), std::bind(&TrailWorker::threadFiniProc, this)),
          gpuDevId_(gpuDevId) {
    }

    void WaitStartOk() {
        cw_.wait();
    }

    future<int32_t> commitAsyncTask(
        int32_t videoChl,
        uint64_t timeStamp,
        const uint8_t *rgbImg,
        uint32_t width,
        uint32_t height,
        const string &param,
        char *jsonRsp,
        uint32_t *rspLen
    ) {
        return tp_.commit(trail, videoChl, timeStamp, rgbImg, width, height, param, jsonRsp, rspLen);
    }

private:
    void threadInitProc() {
        LOG_INFO("Init trail thread {}", getCurrentThreadId());

        int ret = seemmo_thread_init(SEEMMO_LOAD_TYPE_FILTER, gpuDevId_, 1);
        if (0 != ret) {
            LOG_ERROR("Failed to init trail thread, ret {}", ret);
            throw runtime_error("init trail thread error");
        }

        LOG_INFO("Init trail thread {} succeed", getCurrentThreadId());
        cw_.countDown();
    }

    void threadFiniProc() {
        LOG_INFO("Destroy trail thread {}", getCurrentThreadId());
        seemmo_thread_uninit();
        LOG_INFO("Destroy trail thread {} succeed", getCurrentThreadId());
    }

    static int32_t trail(
        int32_t videoChl,
        uint64_t timestamp,
        const uint8_t *bgr24,
        uint32_t width,
        uint32_t height,
        const string &param,
        char *jsonRsp,
        uint32_t *rspLen
    ) {
        const char *p = param.c_str();
        int32_t len = *rspLen;
        int ret = seemmo_video_pvc(1, &videoChl, &timestamp, &bgr24, &height, &width, &p, jsonRsp, len, 2);
        if (0 != ret) {
            LOG_ERROR("Call seemmo_video_pvc fail, ret {}", ret);
            return ret;
        }
        LOG_INFO("TRAIL RSP: {}", jsonRsp);
        *rspLen = len;
        return ERR_OK;
    }

    uint64_t getCurrentThreadId() {
        std::ostringstream oss;
        oss << std::this_thread::get_id();
        std::string stid = oss.str();
        return std::stoull(stid);
    }

private:
    int32_t thrNum_;
    threadpool tp_;
    CountDownLatch cw_;
    uint32_t gpuDevId_;
};

class RecognizeWorker {
public:
    RecognizeWorker(uint32_t gpuDevId)
        : thrNum_(1),
          cw_(thrNum_),
          tp_(thrNum_, std::bind(&RecognizeWorker::threadInitProc, this), std::bind(&RecognizeWorker::threadFiniProc, this)),
          gpuDevId_(gpuDevId) {
    };

    void WaitStartOk() {
        cw_.wait();
    }

    future<uint32_t> CommitAsyncTask(
        const uint8_t *rgbImg,
        uint32_t width,
        uint32_t height,
        const string &param,
        char *jsonRsp,
        uint32_t *rspLen
    ) {
        return tp_.commit(rec, rgbImg, height, width, param, jsonRsp, rspLen);
    }

private:
    void threadInitProc() {
        LOG_INFO("Init recognize thread {}", getCurrentThreadId());

        int ret = seemmo_thread_init(SEEMMO_LOAD_TYPE_RECOG, gpuDevId_, 1);
        if (0 != ret) {
            LOG_ERROR("Failed to init recognize thread, ret {}", ret);
            throw runtime_error("Init recognize thread error");
        }

        LOG_INFO("Init recognize thread {} succeed", getCurrentThreadId());
        cw_.countDown();
    }

    void threadFiniProc() {
        LOG_INFO("Destroy recognize thread {}", getCurrentThreadId());
        seemmo_thread_uninit();
        LOG_INFO("Destroy recognize thread {} succeed", getCurrentThreadId());
    }

    static uint32_t rec(
        const uint8_t *rgbImg,
        uint32_t width,
        uint32_t height,
        const string &param,
        char *jsonRsp,
        uint32_t *rspLen
    ) {
        int32_t len = *rspLen;
        int ret = seemmo_pvc_recog(1, &rgbImg, &height, &width, param.c_str(), jsonRsp, len, 2);
        if (0 != ret) {
            LOG_ERROR("Call seemmo_pvc_recog error, ret {}", ret);
            return ret;
        }
        LOG_INFO("RSP: {}", jsonRsp);
        *rspLen = len;
        return ERR_OK;
    }

    uint64_t getCurrentThreadId() {
        std::ostringstream oss;
        oss << std::this_thread::get_id();
        std::string stid = oss.str();
        return std::stoull(stid);
    }

private:
    int32_t thrNum_;
    threadpool tp_;
    CountDownLatch cw_;
    uint32_t gpuDevId_;
};


class Algo {
public:
    Algo() {
    }

    void Init(const string &basedir, uint32_t imgCoreNum, uint32_t videoCoreNum, const string &authServer,
              uint32_t authType, uint32_t hwDevId) {
        basedir_ = basedir;
        imgCoreNum_ = imgCoreNum;
        videoCoreNum_ = videoCoreNum;
        authServer_ = authServer;
        authType_ = authType;
        hwDevId_ = hwDevId;

        LOG_INFO("Init seemmo sdk, basedir={}, imgCoreNum={}, videoCoreNum={}, authServer={}, authType={}, hwDevId={}",
                 basedir_, imgCoreNum_, videoCoreNum_, authServer_, authType_, hwDevId_);

        int ret = seemmo_process_init(basedir_.c_str(), imgCoreNum_, videoCoreNum_, authServer_.c_str(), authType_, true);
        if (0 != ret) {
            throw runtime_error("init seemmo sdk error, ret " + std::to_string(ret));
        }

        LOG_INFO("Init seemmo sdk succeed");

        trailWorker = std::make_shared<TrailWorker>(hwDevId_);
        recWorker = std::make_shared<RecognizeWorker>(hwDevId_);

        trailWorker->WaitStartOk();
        recWorker->WaitStartOk();
    }

    void Destroy() {
        LOG_INFO("Release resources");
        trailWorker.reset();
        recWorker.reset();
        seemmo_uninit();
        LOG_INFO("Destroy seemmo sdk succeed");
    }

    uint32_t Trail(
        int32_t videoChl, uint64_t timeStamp,
        const uint8_t *bgr24,
        uint32_t height,
        uint32_t width,
        const string &param,
        char *jsonRsp,
        uint32_t *rspLen
    ) {
        auto f = trailWorker->commitAsyncTask(videoChl, timeStamp, bgr24, height, width, param, jsonRsp, rspLen);
        return f.get();
    }

    uint32_t Recognize(
        uint8_t *bgr24,
        uint32_t width,
        uint32_t height,
        const string &param,
        char *jsonRsp,
        uint32_t *rspLen
    ) {
        auto f = recWorker->CommitAsyncTask(bgr24, height, width, param, jsonRsp, rspLen);
        return f.get();
    }

private:
    string basedir_;
    uint32_t imgCoreNum_;
    uint32_t videoCoreNum_;
    string authServer_;
    uint32_t authType_;
    uint32_t hwDevId_;
    shared_ptr<TrailWorker> trailWorker;
    shared_ptr<RecognizeWorker> recWorker;
};

typedef Singleton<Algo> AlgoSingleton;

}
}


using algo::seemmo::AlgoSingleton;

int32_t Seemmo_AlgoInit(const char *basedir, uint32_t imgCoreNum, uint32_t videoCoreNum, const char *authServer,
                        uint32_t authType, uint32_t hwDevId) {
    AlgoSingleton::getInstance().Init(basedir, imgCoreNum, videoCoreNum, authServer, authType, hwDevId);
    return ERR_OK;
}

int32_t Seemmo_AlgoDestroy(void) {
    AlgoSingleton::getInstance().Destroy();
    return ERR_OK;
}

int32_t Seemmo_AlgoTrail(int32_t videoChl, uint64_t timeStamp, const uint8_t *bgr24,
                         uint32_t width, uint32_t height, const char *param, char *jsonRsp, uint32_t &rspLen) {
    return AlgoSingleton::getInstance().Trail(videoChl, timeStamp, bgr24, height, width, param, jsonRsp, &rspLen);
}

int32_t Seemmo_AlgoRecognize(uint8_t *bgr24, uint32_t width, uint32_t height, const char *param, char *jsonRsp,
                             uint32_t &rspLen) {
    return AlgoSingleton::getInstance().Recognize(bgr24, width, height, param, jsonRsp, &rspLen);
}


