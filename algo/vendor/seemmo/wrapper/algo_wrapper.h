#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum ErrType {
    ERR_OK = 0,
    ERR_FAIL = 1,
    ERR_NPE,

    ERR_MAX = 0xffffffff
};

int32_t Seemmo_AlgoInit(const char *basedir,
                        uint32_t workerThrNum,
                        uint32_t imgCoreNum,
                        uint32_t videoCoreNum,
                        const char *authServer,
                        uint32_t authType,
                        uint32_t hwDevId);

int32_t Seemmo_AlgoDestroy(void);

int32_t Seemmo_AlgoTrail(int32_t videoChl,
                         uint64_t timestamp,
                         const uint8_t *bgr24,
                         uint32_t width,
                         uint32_t height,
                         const char *param,
                         char *jsonRsp,
                         uint32_t &rspLen);

int32_t Seemmo_AlgoRecognize(uint8_t *bgr24,
                             uint32_t width,
                             uint32_t height,
                             const char *param,
                             char *jsonRsp,
                             uint32_t &rspLen);

#ifdef __cplusplus
}
#endif // __cplusplus
