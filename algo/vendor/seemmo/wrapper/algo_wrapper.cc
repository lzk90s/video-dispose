#include <iostream>
#include <string>
#include <memory>

#include "common/helper/logger.h"

#include "algo/vendor/seemmo/wrapper/algo_wrapper.h"
#include "algo/vendor/seemmo/wrapper/workers.h"

using namespace std;

namespace algo {
namespace seemmo {


//算法服务
class AlgoController {
public:
    AlgoController() {
    }

    void Init(
        const string &basedir,
        uint32_t imgThrNum,
        uint32_t videoThrNum,
        uint32_t imgCoreNum,
        uint32_t videoCoreNum,
        const string &authServer,
        uint32_t authType,
        uint32_t hwDevId
    ) {
        LOG_INFO("Init seemmo sdk, basedir={}, imgThrNum={}, videoThrNum={}, imgCoreNum={}, videoCoreNum={}, authServer={}, authType={}, hwDevId={}",
                 basedir, imgThrNum, videoThrNum, imgCoreNum, videoCoreNum, authServer, authType, hwDevId);

        // 深瞐sdk进程初始化
        int ret = seemmo_process_init(basedir.c_str(), imgCoreNum, videoCoreNum, authServer.c_str(), authType, true);
        if (0 != ret) {
            throw runtime_error("Init seemmo sdk error, ret " + std::to_string(ret));
        }

        const char *version = seemmo_version();
        LOG_INFO("Init seemmo sdk succeed, version {}", version);

        // 初始化worker
        trailWorker = std::make_shared<TrailWorker>(hwDevId, videoThrNum);
        recWorker = std::make_shared<RecognizeWorker>(hwDevId, imgThrNum);
        //decRecWorker = std::make_shared<DetectRecognizeWorker>(hwDevId, imgThrNum);

        // 等待所有worker启动完成
        trailWorker->WaitStartOk();
        recWorker->WaitStartOk();
        //decRecWorker->WaitStartOk();
    }

    void Destroy() {
        LOG_INFO("Release resources");
        //深瞐sdk需要并发退出，不能串行停，否则会死锁
        trailWorker->Close();
        recWorker->Close();
        //decRecWorker->Close();

        //等待各个worker退出完成
        trailWorker->WaitStopOk();
        recWorker->WaitStopOk();
        //decRecWorker->WaitStopOk();

        // 卸载深瞐sdk
        seemmo_uninit();
        LOG_INFO("Destroy seemmo sdk succeed");
    }

    uint32_t Trail(
        int32_t videoChl,
        uint64_t timestamp,
        const uint8_t *bgr24,
        uint32_t width,
        uint32_t height,
        const string &param,
        char *jsonRsp,
        uint32_t *rspLen
    ) {
        auto f = trailWorker->commitAsyncTask(videoChl, timestamp, bgr24, width, height, param, jsonRsp, rspLen);
        return f.get();
    }

    uint32_t TrailEnd(
        int32_t videoChl,
        const string &param,
        char *jsonRsp,
        uint32_t *rspLen
    ) {
        auto f = trailWorker->commitAsyncEndTask(videoChl, param, jsonRsp, rspLen);
        return f.get();
    }

    uint32_t Recognize(
        const uint8_t *bgr24,
        uint32_t width,
        uint32_t height,
        const string &param,
        char *jsonRsp,
        uint32_t *rspLen
    ) {
        auto f = recWorker->CommitAsyncTask(bgr24, width, height, param, jsonRsp, rspLen);
        return f.get();
    }

    uint32_t DetectRecognize(
        const uint8_t *bgr24,
        uint32_t width,
        uint32_t height,
        const char *param,
        char *jsonRsp,
        uint32_t *rspLen
    ) {
        //auto f = decRecWorker->CommitAsyncTask(bgr24, width, height, param, jsonRsp, rspLen);
        //return f.get();
        return -1;
    }

private:
    shared_ptr<TrailWorker> trailWorker;
    shared_ptr<RecognizeWorker> recWorker;
    shared_ptr<DetectRecognizeWorker> decRecWorker;
};

typedef Singleton<AlgoController> AlgoSingleton;

}
}


using algo::seemmo::AlgoSingleton;

int32_t SeemmoAlgo_Init(
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
    return ERR_OK;
}

int32_t SeemmoAlgo_Destroy(void) {
    AlgoSingleton::getInstance().Destroy();
    return ERR_OK;
}

int32_t SeemmoAlgo_Trail(
    int32_t videoChl,
    uint64_t timestamp,
    const uint8_t *bgr24,
    uint32_t width,
    uint32_t height,
    const char *param,
    char *jsonRsp,
    uint32_t &rspLen
) {
    return AlgoSingleton::getInstance().Trail(videoChl, timestamp, bgr24, width, height, param, jsonRsp, &rspLen);
}

int32_t SeemmoAlgo_TrailEnd(
    int32_t videoChl,
    const char *param,
    char *jsonRsp,
    uint32_t &rspLen
) {
    return AlgoSingleton::getInstance().TrailEnd(videoChl, param, jsonRsp, &rspLen);
}

int32_t SeemmoAlgo_Recognize(
    const uint8_t *bgr24,
    uint32_t width,
    uint32_t height,
    const char *param,
    char *jsonRsp,
    uint32_t &rspLen
) {
    return AlgoSingleton::getInstance().Recognize(bgr24, width, height, param, jsonRsp, &rspLen);
}

int32_t SeemmoAlgo_DetectRecognize(
    const uint8_t *bgr24,
    uint32_t width,
    uint32_t height,
    const char *param,
    char *jsonRsp,
    uint32_t &rspLen
) {
    return AlgoSingleton::getInstance().DetectRecognize(bgr24, width, height, param, jsonRsp, &rspLen);
}
