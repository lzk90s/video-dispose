#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <sstream>

#include "common/helper/threadpool.h"
#include "common/helper/countdownlatch.h"
#include "common/helper/counttimer.h"
#include "common/helper/logger.h"

#include "sdk_export.h"

using namespace std;

namespace algo {
namespace seemmo {


//业务worker基类
class BusinessWorker {
public:
    BusinessWorker(uint32_t gpuDevId, uint32_t algoThrType, uint32_t thrNum)
        : thrNum_(thrNum),
          cwStart_(thrNum_),
          cwStop_(thrNum_),
          gpuDevId_(gpuDevId),
          algoThrType_(algoThrType) {
        tp_.reset(new threadpool(thrNum_,
                                 std::bind(&BusinessWorker::threadInitProc, this), std::bind(&BusinessWorker::threadFiniProc, this), true));
    }

    void Close() {
        tp_->reset();
    }

    void WaitStartOk() {
        cwStart_.wait();
    }

    void WaitStopOk() {
        cwStop_.wait();
    }

private:
    uint64_t getCurrentThreadId() {
        std::ostringstream oss;
        oss << std::this_thread::get_id();
        std::string stid = oss.str();
        return std::stoull(stid);
    }

    void threadInitProc() {
        LOG_INFO("Init thread {}, type [{}]", getCurrentThreadId(), algoThrType_);

        int ret = seemmo_thread_init(algoThrType_, gpuDevId_, 1);
        if (0 != ret) {
            throw runtime_error("Init thread error");
        }
        cwStart_.countDown();
        LOG_INFO("Init thread {} succeed", getCurrentThreadId());
    }

    void threadFiniProc() {
        LOG_INFO("Destroy thread {}", getCurrentThreadId());
        seemmo_thread_uninit();
        cwStop_.countDown();
        LOG_INFO("Destroy thread {} succeed", getCurrentThreadId());
    }

protected:
    int32_t thrNum_;
    unique_ptr<threadpool> tp_;
    CountDownLatch cwStart_;
    CountDownLatch cwStop_;
    uint32_t gpuDevId_;
    uint32_t algoThrType_;
};

// 跟踪worker
class TrailWorker : public BusinessWorker {
public:
    TrailWorker(uint32_t gpuDevId, uint32_t thrNum)
        : BusinessWorker(gpuDevId, SEEMMO_LOAD_TYPE_FILTER, thrNum) {
    }

    future<int32_t> commitAsyncTask(
        int32_t videoChl,
        uint64_t timestamp,
        const uint8_t *bgr24,
        uint32_t width,
        uint32_t height,
        const string &param,
        char *jsonRsp,
        uint32_t *rspLen
    ) {
        return tp_->commit(trail, videoChl, timestamp, bgr24, width, height, param, jsonRsp, rspLen);
    }

    future<int32_t> commitAsyncEndTask(
        int32_t videoChl,
        const string &param,
        char *jsonRsp,
        uint32_t *rspLen
    ) {
        return tp_->commit(trailEnd, videoChl, param, jsonRsp, rspLen);
    }

private:

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
        *rspLen = len;
        return 0;
    }

    static int32_t trailEnd(
        int32_t videoChl,
        const string &param,
        char *jsonRsp,
        uint32_t *rspLen
    ) {
        const char *p = param.c_str();
        int32_t len = *rspLen;
        int ret = seemmo_video_pvc_end(1, &videoChl, &p, jsonRsp, len, 2);
        if (0 != ret) {
            LOG_ERROR("Call seemmo_video_pvc fail, ret {}", ret);
            return ret;
        }
        *rspLen = len;
        return 0;
    }
};

//择优结果识别worker
class RecognizeWorker : public BusinessWorker {
public:
    RecognizeWorker(uint32_t gpuDevId, uint32_t thrNum)
        : BusinessWorker(gpuDevId, SEEMMO_LOAD_TYPE_RECOG, thrNum) {
    }

    future<int32_t> CommitAsyncTask(
        const uint8_t *bgr24,
        uint32_t width,
        uint32_t height,
        const string &param,
        char *jsonRsp,
        uint32_t *rspLen
    ) {
        return tp_->commit(rec, bgr24, width, height, param, jsonRsp, rspLen);
    }

private:

    static int32_t rec(
        const uint8_t *bgr24,
        uint32_t width,
        uint32_t height,
        const string &param,
        char *jsonRsp,
        uint32_t *rspLen
    ) {
        int32_t len = *rspLen;
        int ret = seemmo_pvc_recog(1, &bgr24, &height, &width, param.c_str(), jsonRsp, len, 2);
        if (0 != ret) {
            LOG_ERROR("Call seemmo_pvc_recog error, ret {}", ret);
            return ret;
        }
        *rspLen = len;
        return 0;
    }
};


//图片检测+识别worker
class DetectRecognizeWorker : public BusinessWorker {
public:
    DetectRecognizeWorker(uint32_t gpuDevId, uint32_t thrNum)
        : BusinessWorker(gpuDevId, SEEMMO_LOAD_TYPE_ALL, thrNum) {
    }

    future<int32_t> CommitAsyncTask(
        const uint8_t *bgr24,
        uint32_t width,
        uint32_t height,
        const string &param,
        char *jsonRsp,
        uint32_t *rspLen
    ) {
        return tp_->commit(detectAndRecog, bgr24, width, height, param, jsonRsp, rspLen);
    }

private:

    static int32_t detectAndRecog(
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
        *rspLen = len;
        return 0;
    }
};

}
}