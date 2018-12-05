#pragma once

#include <string>
#include <dlfcn.h>
#include <cstdint>

#include "common/helper/logger.h"

using namespace std;

namespace algo {
namespace gosun_ext {

typedef int32_t (*PF_GosunAlgo_Init)(
    const char *basedir,
    uint32_t imgThrNum,
    uint32_t videoThrNum,
    uint32_t imgCoreNum,
    uint32_t videoCoreNum,
    const char *authServer,
    uint32_t authType,
    uint32_t hwDevId
);

typedef int32_t (*PF_GosunAlgo_Destroy)(void);

typedef int32_t (*PF_GosunAlgo_DetectRecognize)(
    const uint8_t *bgr24,
    uint32_t width,
    uint32_t height,
    const char *param,
    char *jsonRsp,
    uint32_t &rspLen
);


//算法库加载器，用dlopen方式，避免两边依赖了相同的so，但是版本不一致导致问题
class AlgoLoader {
public:
    const char* ALGO_DLL_NAME = "libgosun_ext_wrapper.so";

    AlgoLoader()
        : handle_(nullptr),
          pf_GosunAlgo_Init_(nullptr),
          pf_GosunAlgo_Destroy_(nullptr),
          pf_GosunAlgo_DetectRecognize(nullptr) {
    }

    ~AlgoLoader() {
        Unload();
    }

    void Load(const string &baseDir,
              uint32_t imgThrNum,
              uint32_t videoThrNum,
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

        pf_GosunAlgo_Init_ = (PF_GosunAlgo_Init)dlsym(handle_, "GosunAlgo_Init");
        pf_GosunAlgo_Destroy_ = (PF_GosunAlgo_Destroy)dlsym(handle_, "GosunAlgo_Destroy");
        pf_GosunAlgo_DetectRecognize = (PF_GosunAlgo_DetectRecognize)dlsym(handle_, "GosunAlgo_DetectRecognize");

        if (nullptr == pf_GosunAlgo_Init_ ||
                nullptr == pf_GosunAlgo_Destroy_ ||
                nullptr == pf_GosunAlgo_DetectRecognize) {
            throw runtime_error("invalid symbol in dll");
        }

        int ret = pf_GosunAlgo_Init_(baseDir.c_str(), imgThrNum, videoThrNum, imgCoreNum,videoCoreNum, authServer.c_str(),
                                     authType, gpuDevId);
        if (0 != ret) {
            throw runtime_error("init algorithm error, ret " + std::to_string(ret));
        }
    }

    void Unload() {
        if (nullptr != pf_GosunAlgo_Destroy_ && nullptr != handle_) {
            LOG_INFO("Unload algo");

            pf_GosunAlgo_Destroy_();

            dlclose(handle_);
            handle_ = nullptr;
        }
    }

    int32_t DetectRecognize(const uint8_t *bgr24, uint32_t width, uint32_t height, const string &param, char *jsonRsp,
                            uint32_t &rspLen) {
        if (nullptr == pf_GosunAlgo_DetectRecognize) {
            throw runtime_error("Nullpointer");
        }
        return pf_GosunAlgo_DetectRecognize(bgr24, width, height, param.c_str(), jsonRsp, rspLen);
    }

private:
    void *handle_;
    PF_GosunAlgo_Init pf_GosunAlgo_Init_;
    PF_GosunAlgo_Destroy pf_GosunAlgo_Destroy_;
    PF_GosunAlgo_DetectRecognize pf_GosunAlgo_DetectRecognize;
};
}
}
