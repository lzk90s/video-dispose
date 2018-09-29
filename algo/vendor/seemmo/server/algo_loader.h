#pragma once

#include <string>
#include <dlfcn.h>
#include <cstdint>

#include "common/helper/logger.h"

using namespace std;

namespace algo {
namespace seemmo {

typedef int32_t (*PF_SeemmoAlgo_Init)(
    const char *basedir,
    uint32_t imgThrNum,
    uint32_t videoThrNum,
    uint32_t imgCoreNum,
    uint32_t videoCoreNum,
    const char *authServer,
    uint32_t authType,
    uint32_t hwDevId
);

typedef int32_t (*PF_SeemmoAlgo_Destroy)(void);

typedef int32_t (*PF_SeemmoAlgo_Trail)(
    int32_t videoChl,
    uint64_t timestamp,
    const uint8_t *bgr24,
    uint32_t width,
    uint32_t height,
    const char *param,
    char *jsonRsp,
    uint32_t &rspLen
);

typedef int32_t (*PF_SeemmoAlgo_Recognize)(
    const uint8_t *bgr24,
    uint32_t width,
    uint32_t height,
    const char *param,
    char *jsonRsp,
    uint32_t &rspLen
);

typedef int32_t (*PF_SeemmoAlgo_DetectRecognize)(
    const uint8_t *bgr24,
    uint32_t width,
    uint32_t height,
    const char *param,
    char *jsonRsp,
    uint32_t &rspLen
);


//深瞐算法库加载器，用dlopen方式，是因为深瞐使用的protobuf版本比较低，和我们使用的protobuf版本冲突，
//为了避免冲突，我们的protobuf使用静态链接
class AlgoLoader {
public:
    const char* ALGO_DLL_NAME = "libseemmo_wrapper.so";

    AlgoLoader()
        : handle_(nullptr),
          pf_SeemmoAlgo_Init_(nullptr),
          pf_SeemmoAlgo_Destroy_(nullptr),
          pf_SeemmoAlgo_Trail_(nullptr),
          pf_SeemmoAlgo_Recognize_(nullptr),
          pf_SeemmoAlgo_DetectRecognize(nullptr) {
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

        pf_SeemmoAlgo_Init_ = (PF_SeemmoAlgo_Init)dlsym(handle_, "SeemmoAlgo_Init");
        pf_SeemmoAlgo_Destroy_ = (PF_SeemmoAlgo_Destroy)dlsym(handle_, "SeemmoAlgo_Destroy");
        pf_SeemmoAlgo_Trail_ = (PF_SeemmoAlgo_Trail)dlsym(handle_, "SeemmoAlgo_Trail");
        pf_SeemmoAlgo_Recognize_ = (PF_SeemmoAlgo_Recognize)dlsym(handle_, "SeemmoAlgo_Recognize");
        pf_SeemmoAlgo_DetectRecognize = (PF_SeemmoAlgo_DetectRecognize)dlsym(handle_, "SeemmoAlgo_DetectRecognize");

        if (nullptr == pf_SeemmoAlgo_Init_ ||
                nullptr == pf_SeemmoAlgo_Destroy_ ||
                nullptr == pf_SeemmoAlgo_Trail_   ||
                nullptr == pf_SeemmoAlgo_Recognize_ ||
                nullptr == pf_SeemmoAlgo_DetectRecognize) {
            throw runtime_error("invalid symbol in dll");
        }

        int ret = pf_SeemmoAlgo_Init_(baseDir.c_str(), imgThrNum, videoThrNum, imgCoreNum,videoCoreNum, authServer.c_str(),
                                      authType, gpuDevId);
        if (0 != ret) {
            throw runtime_error("init algorithm error, ret " + std::to_string(ret));
        }
    }

    void Unload() {
        if (nullptr != pf_SeemmoAlgo_Destroy_ && nullptr != handle_) {
            LOG_INFO("Unload algo");

            pf_SeemmoAlgo_Destroy_();

            dlclose(handle_);
            pf_SeemmoAlgo_Init_ = nullptr;
            pf_SeemmoAlgo_Destroy_ = nullptr;
            pf_SeemmoAlgo_Trail_ = nullptr;
            pf_SeemmoAlgo_Recognize_ = nullptr;
            handle_ = nullptr;
        }
    }

    int32_t Trail(int32_t videoChl, uint64_t timestamp, const uint8_t *bgr24, uint32_t width, uint32_t height,
                  const string &param, char *jsonRsp, uint32_t &rspLen) {
        if (nullptr == pf_SeemmoAlgo_Trail_) {
            throw runtime_error("Nullpointer");
        }
        return pf_SeemmoAlgo_Trail_(videoChl, timestamp, bgr24, width, height, param.c_str(), jsonRsp, rspLen);
    }

    int32_t Recognize(const uint8_t *bgr24, uint32_t width, uint32_t height, const string &param, char *jsonRsp,
                      uint32_t &rspLen) {
        if (nullptr == pf_SeemmoAlgo_Recognize_) {
            throw runtime_error("Nullpointer");
        }
        return pf_SeemmoAlgo_Recognize_(bgr24, width, height, param.c_str(), jsonRsp, rspLen);
    }

    int32_t DetectRecognize(const uint8_t *bgr24, uint32_t width, uint32_t height, const string &param, char *jsonRsp,
                            uint32_t &rspLen) {
        if (nullptr == pf_SeemmoAlgo_Recognize_) {
            throw runtime_error("Nullpointer");
        }
        return pf_SeemmoAlgo_DetectRecognize(bgr24, width, height, param.c_str(), jsonRsp, rspLen);
    }

private:
    void *handle_;
    PF_SeemmoAlgo_Init pf_SeemmoAlgo_Init_;
    PF_SeemmoAlgo_Destroy pf_SeemmoAlgo_Destroy_;
    PF_SeemmoAlgo_Trail pf_SeemmoAlgo_Trail_;
    PF_SeemmoAlgo_Recognize pf_SeemmoAlgo_Recognize_;
    PF_SeemmoAlgo_DetectRecognize pf_SeemmoAlgo_DetectRecognize;
};
}
}
