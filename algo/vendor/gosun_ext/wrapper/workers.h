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

using namespace std;

namespace algo {
namespace gosun_ext {


//业务worker基类
class BusinessWorker {
public:
    BusinessWorker(uint32_t gpuDevId, uint32_t algoThrType, uint32_t thrNum)
        : thrNum_(thrNum),
          cwStart_(thrNum_),
          cwStop_(thrNum_),
          gpuDevId_(gpuDevId),
          algoThrType_(algoThrType) {
        for (uint32_t i = 0; i < thrNum; i++) {
            shared_ptr<threadpool> w(
                new threadpool(1,std::bind(&BusinessWorker::threadInitProc, this),
                               std::bind(&BusinessWorker::threadFiniProc, this))
            );
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

    shared_ptr<threadpool> ChooseExecutor(int32_t seed) {
        uint32_t idx = 0;
        if (seed < 0) {
            //seed小于0时，随机选取一个
            auto t = std::chrono::time_point_cast<std::chrono::milliseconds>
                     (std::chrono::steady_clock::now()).time_since_epoch().count();
            idx = t % thrNum_;
        } else {
            idx = seed % thrNum_;
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
    int32_t thrNum_;
    vector<shared_ptr<threadpool>> executors_;
    CountDownLatch cwStart_;
    CountDownLatch cwStop_;
    uint32_t gpuDevId_;
    uint32_t algoThrType_;
};

//图片检测+识别worker
class DetectRecognizeWorker : public BusinessWorker {
public:
    DetectRecognizeWorker(uint32_t gpuDevId, uint32_t thrNum)
        : BusinessWorker(gpuDevId, 3, thrNum) {
    }

    future<int32_t> CommitAsyncTask(
        const uint8_t *bgr24,
        uint32_t width,
        uint32_t height,
        const string &param,
        char *jsonRsp,
        uint32_t *rspLen
    ) {
        return ChooseExecutor(-1)->commit(detectAndRecog, bgr24, width, height, param, jsonRsp, rspLen);
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
        char *rspMsg = gosun_facecheck(param.c_str(), (unsigned char*)rgbImg, height, width);
        if (nullptr == rspMsg) {
            LOG_ERROR("Call gosun_facecheck error");
            return -1;
        }
        *rspLen = strlen(rspMsg);
        strncpy(jsonRsp, rspMsg, strlen(rspMsg));
        free(rspMsg);
        return 0;
    }
};

}
}