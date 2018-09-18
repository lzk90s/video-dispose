#pragma once

#include <string>
#include <dlfcn.h>
#include <cstdint>

using namespace std;

namespace algo {
namespace seemmo {

typedef int32_t(*pf_Algo_Init)(const char *cfg);

typedef int32_t(*pf_Algo_Destroy)(void);

typedef int32_t(*pt_Algo_VideoTrailAndRec)(int32_t videoChl, uint64_t timeStamp, const uint8_t *rgbImg, uint32_t height,
        uint32_t width);


class AlgoLoader {
public:
    const char* ALGO_DLL_NAME = "libseemmo_wrapper.so";

    AlgoLoader()
        : handle_(nullptr), pfAlgoInit_(nullptr), pfAlgoDestroy_(nullptr), pfAlgoVideoTrailAndRec_(nullptr) {
    }

    ~AlgoLoader() {
        if (nullptr != handle_) {
            dlclose(handle_);
            pfAlgoInit_ = nullptr;
            pfAlgoDestroy_ = nullptr;
            pfAlgoVideoTrailAndRec_ = nullptr;
            handle_ = nullptr;
        }
    }

    void Load(const string &algoCfgPath) {
        handle_ = dlopen(ALGO_DLL_NAME, RTLD_LAZY);
        if (nullptr == handle_) {
            throw runtime_error("Load ALGO dll error, " + std::string(dlerror()));
        }

        pfAlgoInit_ = (pf_Algo_Init)dlsym(handle_, "Algo_Init");
        pfAlgoDestroy_ = (pf_Algo_Destroy)dlsym(handle_, "Algo_Destroy");
        pfAlgoVideoTrailAndRec_ = (pt_Algo_VideoTrailAndRec)dlsym(handle_, "Algo_VideoTrailAndRec");

        if (nullptr == pfAlgoInit_ || nullptr == pfAlgoDestroy_ || nullptr == pfAlgoVideoTrailAndRec_) {
            throw runtime_error("Invalid symbol in dll");
        }

        int ret = pfAlgoInit_(algoCfgPath.c_str());
        if (0 != ret) {
            throw runtime_error("Init algorithm error, ret " + std::to_string(ret));
        }
    }

    int32_t Algo_VideoTrailAndRec(int32_t videoChl, uint64_t timeStamp, const uint8_t *rgbImg, uint32_t height,
                                  uint32_t width) {
        if (nullptr == pfAlgoVideoTrailAndRec_) {
            throw runtime_error("Nullpointer");
        }
        return pfAlgoVideoTrailAndRec_(videoChl, timeStamp, rgbImg, height, width);
    }

private:
    void *handle_;
    pf_Algo_Init pfAlgoInit_;
    pf_Algo_Destroy pfAlgoDestroy_;
    pt_Algo_VideoTrailAndRec pfAlgoVideoTrailAndRec_;
};
}
}
