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

//��a�㷨��ʼ��
int32_t SeemmoAlgo_Init(
    const char *basedir,
    uint32_t workerThrNum,
    uint32_t imgCoreNum,
    uint32_t videoCoreNum,
    const char *authServer,
    uint32_t authType,
    uint32_t hwDevId
);

//��a�㷨����
int32_t SeemmoAlgo_Destroy(void);

//��Ƶ����+���
int32_t SeemmoAlgo_Trail(
    int32_t videoChl,
    uint64_t timestamp,
    const uint8_t *bgr24,
    uint32_t width,
    uint32_t height,
    const char *param,
    char *jsonRsp,
    uint32_t &rspLen
);

//��Ƶͼ��ʶ��
int32_t SeemmoAlgo_Recognize(
    const uint8_t *bgr24,
    uint32_t width,
    uint32_t height,
    const char *param,
    char *jsonRsp,
    uint32_t &rspLen
);

//���+ʶ��
int32_t SeemmoAlgo_DetectRecognize(
    const uint8_t *bgr24,
    uint32_t width,
    uint32_t height,
    const char *param,
    char *jsonRsp,
    uint32_t &rspLen
);

#ifdef __cplusplus
}
#endif // __cplusplus
