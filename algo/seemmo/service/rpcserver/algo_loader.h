#pragma once

#include <string>
#include <dlfcn.h>
#include <cstdint>

using namespace std;

typedef int32_t (*pf_Algo_Init)(const char *cfg);

typedef int32_t (*pf_Algo_Destroy)(void);

typedef int32_t (*pt_Algo_VideoTrailAndRec)(int32_t videoChl, uint64_t timeStamp, const uint8_t *rgbImg, uint32_t height, uint32_t width);

class AlgoLoader {
public:
    const char* ALGO_DLL_NAME = "libseemmo_vendor.so";

    AlgoLoader()
        : handle(nullptr), pfAlgoInit(nullptr), pfAlgoDestroy(nullptr), pfAlgoVideoTrailAndRec(nullptr) {
    }

    ~AlgoLoader() {
        if (nullptr != handle) {
            dlclose(handle);
            pfAlgoInit = nullptr;
            pfAlgoDestroy = nullptr;
            pfAlgoVideoTrailAndRec = nullptr;
            handle = nullptr;
        }
    }

    void Load(const string &algoCfgPath) {
        handle = dlopen(ALGO_DLL_NAME, RTLD_LAZY);
        if (nullptr == handle) {
            throw runtime_error("Load ALGO dll error, " + std::string(dlerror()));
        }

        pfAlgoInit = (pf_Algo_Init)dlsym(handle, "Algo_Init");
        pfAlgoDestroy = (pf_Algo_Destroy)dlsym(handle, "Algo_Destroy");
        pfAlgoVideoTrailAndRec = (pt_Algo_VideoTrailAndRec)dlsym(handle, "Algo_VideoTrailAndRec");

        if (nullptr == pfAlgoInit || nullptr == pfAlgoDestroy || nullptr == pfAlgoVideoTrailAndRec) {
            throw runtime_error("Invalid symbol in dll");
        }

        int ret = pfAlgoInit(algoCfgPath.c_str());
        if (0 != ret) {
            throw runtime_error("Init algorithm error, ret " + std::to_string(ret));
        }
    }

    int32_t Algo_VideoTrailAndRec(int32_t videoChl, uint64_t timeStamp, const uint8_t *rgbImg, uint32_t height, uint32_t width) {
        if (nullptr == pfAlgoVideoTrailAndRec) {
            throw runtime_error("Nullpointer");
        }
        return pfAlgoVideoTrailAndRec(videoChl, timeStamp, rgbImg, height, width);
    }

private:
    void *handle;
    pf_Algo_Init pfAlgoInit;
    pf_Algo_Destroy pfAlgoDestroy;
    pt_Algo_VideoTrailAndRec pfAlgoVideoTrailAndRec;
};
