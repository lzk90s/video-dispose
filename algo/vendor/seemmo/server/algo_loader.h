#pragma once

#include <string>
#include <dlfcn.h>
#include <cstdint>

#include "common/helper/logger.h"

using namespace std;

namespace algo {
namespace seemmo {

typedef int32_t (*PF_Seemmo_AlgoInit)(const char *basedir,
                                      uint32_t imgCoreNum,
                                      uint32_t videoCoreNum,
                                      const char *authServer,
                                      uint32_t authType,
                                      uint32_t hwDevId);

typedef int32_t (*PF_Seemmo_AlgoDestroy)(void);

typedef int32_t (*PF_Seemmo_AlgoTrail)(int32_t videoChl,
                                       uint64_t timestamp,
                                       const uint8_t *bgr24,
                                       uint32_t width,
                                       uint32_t height,
                                       const char *param,
                                       char *jsonRsp,
                                       uint32_t &rspLen);

typedef int32_t (*PF_Seemmo_AlgoRecognize)(const uint8_t *bgr24,
        uint32_t width,
        uint32_t height,
        const char *param,
        char *jsonRsp,
        uint32_t &rspLen);


class AlgoLoader {
public:
    const char* ALGO_DLL_NAME = "libseemmo_wrapper.so";

    AlgoLoader()
        : handle_(nullptr),
          pf_Seemmo_AlgoInit_(nullptr),
          pf_Seemmo_AlgoDestroy_(nullptr),
          pf_Seemmo_AlgoTrail_(nullptr),
          pf_Seemmo_AlgoRecognize_(nullptr) {
    }

    ~AlgoLoader() {
        Unload();
    }

    void Load(const string &baseDir,
              uint32_t imgCoreNum,
              uint32_t videoCoreNum,
              uint32_t authType,
              const string &authServer,
              uint32_t gpuDevId) {
        LOG_INFO("Load algo {}", ALGO_DLL_NAME);

        handle_ = dlopen(ALGO_DLL_NAME, RTLD_LAZY);
        if (nullptr == handle_) {
            throw runtime_error("Load ALGO dll error, " + std::string(dlerror()));
        }

        pf_Seemmo_AlgoInit_ = (PF_Seemmo_AlgoInit)dlsym(handle_, "Seemmo_AlgoInit");
        pf_Seemmo_AlgoDestroy_ = (PF_Seemmo_AlgoDestroy)dlsym(handle_, "Seemmo_AlgoDestroy");
        pf_Seemmo_AlgoTrail_ = (PF_Seemmo_AlgoTrail)dlsym(handle_, "Seemmo_AlgoTrail");
        pf_Seemmo_AlgoRecognize_ = (PF_Seemmo_AlgoRecognize)dlsym(handle_, "Seemmo_AlgoRecognize");

        if (nullptr == pf_Seemmo_AlgoInit_ || nullptr == pf_Seemmo_AlgoDestroy_ || nullptr == pf_Seemmo_AlgoTrail_
                || nullptr == pf_Seemmo_AlgoRecognize_) {
            throw runtime_error("invalid symbol in dll");
        }

        int ret = pf_Seemmo_AlgoInit_(baseDir.c_str(), imgCoreNum,videoCoreNum, authServer.c_str(),authType,gpuDevId);
        if (0 != ret) {
            throw runtime_error("init algorithm error, ret " + std::to_string(ret));
        }
    }

    void Unload() {
        if (nullptr != pf_Seemmo_AlgoDestroy_ && nullptr != handle_) {
            LOG_INFO("Unload algo");

            pf_Seemmo_AlgoDestroy_();

            dlclose(handle_);
            pf_Seemmo_AlgoInit_ = nullptr;
            pf_Seemmo_AlgoDestroy_ = nullptr;
            pf_Seemmo_AlgoTrail_ = nullptr;
            pf_Seemmo_AlgoRecognize_ = nullptr;
            handle_ = nullptr;
        }
    }

    int32_t Trail(int32_t videoChl, uint64_t timestamp, const uint8_t *bgr24, uint32_t width, uint32_t height,
                  const string &param, char *jsonRsp, uint32_t &rspLen) {
        if (nullptr == pf_Seemmo_AlgoTrail_) {
            throw runtime_error("Nullpointer");
        }
        return pf_Seemmo_AlgoTrail_(videoChl, timestamp, bgr24, width, height, param.c_str(), jsonRsp, rspLen);
    }

    int32_t Recognize(const uint8_t *bgr24, uint32_t width, uint32_t height, const string &param, char *jsonRsp,
                      uint32_t &rspLen) {
        if (nullptr == pf_Seemmo_AlgoRecognize_) {
            throw runtime_error("Nullpointer");
        }
        return pf_Seemmo_AlgoRecognize_(bgr24, width, height, param.c_str(), jsonRsp, rspLen);
    }

private:
    void *handle_;
    PF_Seemmo_AlgoInit pf_Seemmo_AlgoInit_;
    PF_Seemmo_AlgoDestroy pf_Seemmo_AlgoDestroy_;
    PF_Seemmo_AlgoTrail pf_Seemmo_AlgoTrail_;
    PF_Seemmo_AlgoRecognize pf_Seemmo_AlgoRecognize_;
};
}
}
