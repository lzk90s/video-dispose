#include <iostream>
#include <string>
#include <memory>

#include "common/helper/logger.h"
#include "common/helper/counttimer.h"

#include "algo/vendor/gosun_ext/wrapper/algo_wrapper.h"
#include "algo/vendor/gosun_ext/wrapper/workers.h"

#include "gosun_face_api.h"

namespace video {
namespace algo {
namespace gosun_ext {


//算法服务
class AlgoController {
public:
    AlgoController() {
    }

    void Init(
        const std::string &basedir,
        uint32_t imgThrNum,
        uint32_t videoThrNum,
        uint32_t imgCoreNum,
        uint32_t videoCoreNum,
        const std::string &authServer,
        uint32_t authType,
        uint32_t hwDevId
    ) {
        LOG_INFO("Init gosun sdk, basedir={}, imgThrNum={}, hwDevId={}", basedir, imgThrNum, hwDevId);

        gosun_face_api_init(imgThrNum);

        LOG_INFO("Init gosun sdk succeed");

        // 初始化worker
        decRecWorker = std::make_shared<DetectRecognizeWorker>(hwDevId, imgThrNum);

        // 等待所有worker启动完成
        decRecWorker->WaitStartOk();
    }

    void Destroy() {
        LOG_INFO("Release resources");

        decRecWorker->Close();

        //等待各个worker退出完成
        decRecWorker->WaitStopOk();

        gosun_face_api_deinit();

        LOG_INFO("Destroy gosun sdk succeed");
    }

    uint32_t DetectRecognize(
        const uint8_t *bgr24,
        uint32_t width,
        uint32_t height,
        const char *param,
        char *jsonRsp,
        uint32_t *rspLen
    ) {
        return decRecWorker->CommitAsyncTask(bgr24, width, height, param, jsonRsp, rspLen).get();
    }

private:
    std::shared_ptr<DetectRecognizeWorker> decRecWorker;
};

typedef Singleton<AlgoController> AlgoSingleton;

}
}
}

using video::algo::gosun_ext::AlgoSingleton;

int32_t GosunAlgo_Init(
    const char *basedir,
    uint32_t imgThrNum,
    uint32_t videoThrNum,
    uint32_t imgCoreNum,
    uint32_t videoCoreNum,
    const char *authServer,
    uint32_t authType,
    uint32_t hwDevId
) {
    AlgoSingleton::getInstance().Init(basedir, imgThrNum, videoThrNum, imgCoreNum, videoCoreNum, authServer, authType,
                                      hwDevId);
    return 0;
}

int32_t GosunAlgo_Destroy(void) {
    AlgoSingleton::getInstance().Destroy();
    return 0;
}

int32_t GosunAlgo_DetectRecognize(
    const uint8_t *bgr24,
    uint32_t width,
    uint32_t height,
    const char *param,
    char *jsonRsp,
    uint32_t &rspLen
) {
    CountTimer timer("GosunAlgo_DetectRecognize", 80 * 1000);
    return AlgoSingleton::getInstance().DetectRecognize(bgr24, width, height, param, jsonRsp, &rspLen);
}
