#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

namespace algo {
namespace seemmo {

enum ErrType {
    E_OK = 0,
    E_NPE = 1,	//null pointer
    E_FILEOPEN = 2,
    E_INVALID_CFG = 3,
    E_JSON = 4,

    E_MAX = 0xffffffff
};

#define INVOKE_RETURN_IF_FAIL(method_call, msg) do { \
        int __result=0; \
        if (( __result= (method_call)) != 0) { \
            std::cout<<"Call sdk error, result [" << __result << "] " << msg << endl; \
            return __result; \
                        } \
            } while (0)

int32_t Algo_Init(const char *cfg);

int32_t Algo_Destroy(void);

int32_t Algo_VideoTrailAndRec(int32_t videoChl, uint64_t timeStamp, const uint8_t *rgbImg, uint32_t height,
                              uint32_t width, char *rspJson, uint32_t &rspLen);

}
}
#ifdef __cplusplus
}
#endif // __cplusplus
