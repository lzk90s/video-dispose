#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <sstream>

#include "common/helper/threadpool.h"
#include "common/helper/countdownlatch.h"
#include "common/helper/counttimer.h"
#include "common/helper/logger.h"

#include "gosun_face_api.h"


namespace video {
namespace algo {
namespace gosun_ext {


//业务worker基类
class BusinessWorker {
public:
    BusinessWorker(uint32_t gpuDevId, uint32_t algoThrType, uint32_t thrGroupNum, uint32_t thrNumEachGroup)
        : thrGroupNum_(thrGroupNum),
          cwStart_(thrGroupNum*thrNumEachGroup),
          cwStop_(thrGroupNum*thrNumEachGroup),
          gpuDevId_(gpuDevId),
          algoThrType_(algoThrType) {
        for (uint32_t i = 0; i < thrGroupNum; i++) {
            std::shared_ptr<std::threadpool> w( new std::threadpool(thrNumEachGroup,
                                                std::bind(&BusinessWorker::threadInitProc, this),
                                                std::bind(&BusinessWorker::threadFiniProc, this)));
            executors_.push_back(w);
        }
    }

    virtual ~BusinessWorker() {
        executors_.clear();
    }

    void Close() {
        executors_.clear();
    }

    void WaitStartOk() {
        cwStart_.wait();
    }

    void WaitStopOk() {
        cwStop_.wait();
    }

    std::shared_ptr<std::threadpool> SelectExecutorGroup(int32_t seed) {
        uint32_t idx = 0;
        if (seed < 0) {
            //seed小于0时，随机选取一个
            auto t = std::chrono::time_point_cast<std::chrono::milliseconds>
                     (std::chrono::steady_clock::now()).time_since_epoch().count();
            idx = t % thrGroupNum_;
        } else {
            idx = seed % thrGroupNum_;
        }
        return executors_[idx];
    }

private:
    uint64_t getCurrentThreadId() {
        std::ostringstream oss;
        oss << std::this_thread::get_id();
        std::string stid = oss.str();
        return std::stoull(stid);
    }

    void threadInitProc() {
        // sdk进程初始化
        LOG_INFO("Succeed to init gosun face api thread, tid {}", getCurrentThreadId());
        cwStart_.countDown();
    }

    void threadFiniProc() {
        LOG_INFO("Succeed to destroy gosun face api thread, tid {}", getCurrentThreadId());
        cwStop_.countDown();
    }

protected:
    uint32_t thrGroupNum_;
    std::vector<std::shared_ptr<std::threadpool>> executors_;
    CountDownLatch cwStart_;
    CountDownLatch cwStop_;
    uint32_t gpuDevId_;
    uint32_t algoThrType_;
};

//图片检测+识别worker
class DetectRecognizeWorker : public BusinessWorker {
public:
    DetectRecognizeWorker(uint32_t gpuDevId, uint32_t thrNum)
        : BusinessWorker(gpuDevId, 3, 1, thrNum) {
    }

    std::future<int32_t> CommitAsyncTask(
        const uint8_t *bgr24,
        uint32_t width,
        uint32_t height,
        const std::string &param,
        char *jsonRsp,
        uint32_t *rspLen
    ) {
        return SelectExecutorGroup(-1)->commit(detectAndRecog, bgr24, width, height, param, jsonRsp, rspLen);
    }

private:

    static int32_t detectAndRecog(
        const uint8_t *rgbImg,
        uint32_t width,
        uint32_t height,
        const std::string &param,
        char *jsonRsp,
        uint32_t *rspLen
    ) {
        int len = *rspLen;
        int ret = gosun_facecheck(param.c_str(), (unsigned char*)rgbImg, height, width, jsonRsp, len);
        if (0 != ret) {
            LOG_ERROR("Call gosun_facecheck error");
            return ret;
        }
        *rspLen = len;
        return 0;
    }
};

}
}
}