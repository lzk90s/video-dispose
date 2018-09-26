#include <iostream>
#include <string>
#include <memory>
#include <sstream>

#include "common/helper/logger.h"

#include "sdk_export/sdk_export.h"

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

    void Init(const string &basedir, uint32_t imgCoreNum, uint32_t videoCoreNum, const string &authServer,
              uint32_t authType, uint32_t hwDevId) {
        basedir_ = basedir;
        imgCoreNum_ = imgCoreNum;
        videoCoreNum_ = videoCoreNum;
        authServer_ = authServer;
        authType_ = authType;
        gpuDevId_ = hwDevId;

        LOG_INFO("Init seemmo sdk, basedir={}, imgCoreNum={}, videoCoreNum={}, authServer={}, authType={}, hwDevId={}",
                 basedir_, imgCoreNum_, videoCoreNum_, authServer_, authType_, gpuDevId_);

        // 深瞐sdk进程初始化
        int ret = seemmo_process_init(basedir_.c_str(), imgCoreNum_, videoCoreNum_, authServer_.c_str(), authType_, true);
        if (0 != ret) {
            throw runtime_error("init seemmo sdk error, ret " + std::to_string(ret));
        }

        LOG_INFO("Init seemmo sdk succeed");

        // 初始化worker
        trailWorker = std::make_shared<TrailWorker>(gpuDevId_, videoCoreNum_);
        recWorker = std::make_shared<RecognizeWorker>(gpuDevId_, imgCoreNum_);

        // 等待所有worker启动完成
        trailWorker->WaitStartOk();
        recWorker->WaitStartOk();
    }

    void Destroy() {
        LOG_INFO("Release resources");
        //深瞐sdk需要并发退出，不能串行停，否则会死锁
        trailWorker->Close();
        recWorker->Close();

        //等待各个worker退出完成
        trailWorker->WaitStopOk();
        recWorker->WaitStopOk();

        // 卸载深瞐sdk
        seemmo_uninit();
        LOG_INFO("Destroy seemmo sdk succeed");
    }

    uint32_t Trail(
        int32_t videoChl, uint64_t timeStamp,
        const uint8_t *bgr24,
        uint32_t height,
        uint32_t width,
        const string &param,
        char *jsonRsp,
        uint32_t *rspLen
    ) {
        auto f = trailWorker->commitAsyncTask(videoChl, timeStamp, bgr24, height, width, param, jsonRsp, rspLen);
        return f.get();
    }

    uint32_t Recognize(
        uint8_t *bgr24,
        uint32_t width,
        uint32_t height,
        const string &param,
        char *jsonRsp,
        uint32_t *rspLen
    ) {
        auto f = recWorker->CommitAsyncTask(bgr24, height, width, param, jsonRsp, rspLen);
        return f.get();
    }

private:
    string basedir_;
    uint32_t imgCoreNum_;
    uint32_t videoCoreNum_;
    string authServer_;
    uint32_t authType_;
    uint32_t gpuDevId_;
    shared_ptr<TrailWorker> trailWorker;
    shared_ptr<RecognizeWorker> recWorker;
};

typedef Singleton<AlgoController> AlgoSingleton;

}
}


using algo::seemmo::AlgoSingleton;

int32_t Seemmo_AlgoInit(const char *basedir, uint32_t imgCoreNum, uint32_t videoCoreNum, const char *authServer,
                        uint32_t authType, uint32_t hwDevId) {
    AlgoSingleton::getInstance().Init(basedir, imgCoreNum, videoCoreNum, authServer, authType, hwDevId);
    return ERR_OK;
}

int32_t Seemmo_AlgoDestroy(void) {
    AlgoSingleton::getInstance().Destroy();
    return ERR_OK;
}

int32_t Seemmo_AlgoTrail(int32_t videoChl, uint64_t timeStamp, const uint8_t *bgr24,
                         uint32_t width, uint32_t height, const char *param, char *jsonRsp, uint32_t &rspLen) {
    return AlgoSingleton::getInstance().Trail(videoChl, timeStamp, bgr24, height, width, param, jsonRsp, &rspLen);
}

int32_t Seemmo_AlgoRecognize(uint8_t *bgr24, uint32_t width, uint32_t height, const char *param, char *jsonRsp,
                             uint32_t &rspLen) {
    return AlgoSingleton::getInstance().Recognize(bgr24, width, height, param, jsonRsp, &rspLen);
}


