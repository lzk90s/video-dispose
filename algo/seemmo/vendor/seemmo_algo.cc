#include <iostream>
#include <string>
#include <memory>

#include "common/helper/threadpool.h"
#include "common/helper/countdownlatch.h"
#include "common/helper/counttimer.h"
#include "common/helper/logger.h"

#include "sdk_export/sdk_export.h"
#include "algo/seemmo/vendor/setting.h"
#include "algo/seemmo/vendor/algo.h"
#include "algo/seemmo/vendor/filter_param_builder.h"
#include "algo/seemmo/vendor/filter_result_parser.h"
#include "algo/seemmo/vendor/rec_param_builder.h"
#include "algo/seemmo/vendor/rec_result_parser.h"

using json = nlohmann::json;
using namespace std;

class TrailWorker {
public:
    TrailWorker()
        : cw(1), tp(1, std::bind(&TrailWorker::threadInitProc, this), std::bind(&TrailWorker::threadFiniProc, this)) {
    }

    void WaitStartOk() {
        cw.wait();
    }

    future<shared_ptr<trail::TrailResponseMsg>> commitAsyncTask(int32_t videoChl, uint64_t timeStamp, const uint8_t *rgbImg, uint32_t height, uint32_t width, const string &param) {
        return tp.commit(trail, videoChl, timeStamp, rgbImg, height, width, param);
    }

private:
    void threadInitProc() {
        LOG_INFO("Begin to init trail thread");

        int ret = seemmo_thread_init(SEEMMO_LOAD_TYPE_FILTER, ALGO_SETTING().seemideoCfg.devId, 1);
        if (0 != ret) {
            LOG_ERROR("Failed to init trail thread, ret {}", ret);
        }

        LOG_INFO("Succeed to init trail thread");
        cw.countDown();
    }

    void threadFiniProc() {
        seemmo_thread_uninit();
        LOG_INFO("Succeed to destroy trail thread");
    }

    static shared_ptr<trail::TrailResponseMsg> trail(int32_t videoChl, uint64_t timeStamp, const uint8_t *rgbImg, uint32_t height, uint32_t width,  const string &param) {
        const int BUF_LEN = 1024 * 1024 * 10;
        char* rspBuf = new char[BUF_LEN] {0};
        int32_t rspLen = BUF_LEN;
        const char *p = param.c_str();

        shared_ptr<trail::TrailResponseMsg> rspMsg = make_shared<trail::TrailResponseMsg>();
        int ret = seemmo_video_pvc(1, &videoChl, &timeStamp, &rgbImg, &height, &width, &p, rspBuf, rspLen, 2);
        if (0 != ret) {
            LOG_ERROR("Seemmo video PVC error, ret {}", ret);
            rspMsg->Code = ret;
            return rspMsg;
        }

        trail::FilterResponseParser().Parse(rspBuf, *rspMsg.get());
        LOG_DEBUG("TRAIL: {}", rspBuf);
        return rspMsg;
    }

private:
    threadpool tp;
    CountDownLatch cw;
};

class RecWorker {
public:
    RecWorker()
        : cw(1), tp(1, std::bind(&RecWorker::threadInitProc, this), std::bind(&RecWorker::threadFiniProc, this)) {
    };

    void WaitStartOk() {
        cw.wait();
    }

    future<shared_ptr<rec::RecResponseMsg>> CommitAsyncTask(const uint8_t *rgbImg, uint32_t height, uint32_t width, const string &param) {
        return tp.commit(rec, rgbImg, height, width, param);
    }

private:
    void threadInitProc() {
        LOG_INFO("Begin to init recognize thread");

        int ret = seemmo_thread_init(SEEMMO_LOAD_TYPE_RECOG, ALGO_SETTING().seemideoCfg.devId, 1);
        if (0 != ret) {
            LOG_ERROR("Failed to init recognize thread, ret {}", ret);
        }

        LOG_INFO("Succeed to init recognize thread");
        cw.countDown();
    }

    void threadFiniProc() {
        seemmo_thread_uninit();
        LOG_INFO("Succeed to destroy recognize thread");
    }

    static shared_ptr<rec::RecResponseMsg> rec(const uint8_t *rgbImg, uint32_t height, uint32_t width, const string &param) {
        const int BUF_LEN = 1024 * 1024 * 10;
        char* rspBuf = new char[BUF_LEN] {0};
        int32_t rspLen = BUF_LEN;

        LOG_INFO("REC_PARAM: {}", param.c_str());

        //cout << "REC PARAM : " << param << endl;
        shared_ptr<rec::RecResponseMsg> rspMsg = make_shared<rec::RecResponseMsg>();
        int ret = seemmo_pvc_recog(1, &rgbImg, &height, &width, param.c_str(), rspBuf, rspLen, 2);
        if (0 != ret) {
            LOG_ERROR("Seemmo image error, ret {}", ret);
            rspMsg->Code = ret;
            return rspMsg;
        }

        rec::RecResultParser().Parse(rspBuf, *rspMsg.get());
        LOG_INFO("REC: {}", rspBuf);
        return rspMsg;
    }

private:
    threadpool tp;
    CountDownLatch cw;
};


class SeemideoAlgo {
public:
    SeemideoAlgo() {
    }

    int32_t Init(const string &cfg) {
        ALGO_SETTING().Init(cfg);

        LOG_INFO("Begin to init seemmo sdk");

        INVOKE_RETURN_IF_FAIL(
            seemmo_process_init(ALGO_SETTING().seemideoCfg.baseDir.c_str(),
                                ALGO_SETTING().seemideoCfg.imgCoreNum,
                                ALGO_SETTING().seemideoCfg.videoCoreNum,
                                ALGO_SETTING().seemideoCfg.authServer.c_str(),
                                ALGO_SETTING().seemideoCfg.authType,
                                true),
            "Seemmo process init"
        );

        LOG_INFO("Succeed to init seemmo sdk");

        trailWorker = std::make_shared<TrailWorker>();
        recWorker = std::make_shared<RecWorker>();

        trailWorker->WaitStartOk();
        recWorker->WaitStartOk();

        return E_OK;
    }

    int32_t Destroy() {
        LOG_INFO("Destroy seemmo sdk");
        trailWorker.reset();
        recWorker.reset();
        return seemmo_uninit();
    }

    int32_t TrailAndRec(int32_t videoChl, uint64_t timeStamp, const uint8_t *rgbImg, uint32_t height, uint32_t width) {
        CountTimer timer("TrailAndRec");

        future<shared_ptr<trail::TrailResponseMsg>> f1 = trailWorker->commitAsyncTask(videoChl, timeStamp, rgbImg, height, width, trail::FilterParamBuilder().Build());
        shared_ptr<trail::TrailResponseMsg> trailRsp = f1.get();
        if (nullptr == trailRsp) {
            LOG_ERROR("Null trail response");
            return E_FAIL;
        }

        if (trailRsp->Code != 0) {
            LOG_ERROR("Seemmo trail return error {}, {}", trailRsp->Code, trailRsp->Message);
            return trailRsp->Code;
        }

        if (trailRsp->FilterResults.empty()) {
            LOG_DEBUG("No trail object, ignore");
            return E_OK;
        }

        vector<rec::RecLocation> locations;
        for (auto r : trailRsp->FilterResults) {
            for (auto t : r.Bikes) {
                rec::RecLocation lc;
                trailItem2RecLocation(t, lc);
                locations.push_back(lc);
            }
            for (auto t : r.Vehicles) {
                rec::RecLocation lc;
                trailItem2RecLocation(t, lc);
                locations.push_back(lc);
            }
            for (auto t : r.Pedestrains) {
                rec::RecLocation lc;
                trailItem2RecLocation(t, lc);
                locations.push_back(lc);
            }
        }

        if (locations.empty()) {
            LOG_DEBUG("Trail location is empty, ignore");
            return E_OK;
        }

        future<shared_ptr<rec::RecResponseMsg>> f2 = recWorker->CommitAsyncTask(rgbImg, height, width, rec::RecParamBuilder().Build(locations));
        shared_ptr<rec::RecResponseMsg> recRsp = f2.get();
        if (nullptr == recRsp) {
            LOG_ERROR("Null recognize response");
            return E_FAIL;
        }

        if (recRsp->Code != 0) {
            LOG_ERROR("Seemmo recognize return error {}, {}", recRsp->Code,  recRsp->Message);
            return recRsp->Code;
        }

        return E_OK;
    }

private:
    void trailItem2RecLocation(trail::TrailItem &t, rec::RecLocation &r) {
        r.Type = t.Type;
        r.ContextCode = t.ContextCode;
        r.Trail = t.Trail;
        r.GUID = t.GUID;
        if (0 == t.Detect.Code && !t.Detect.Body.Rect.empty()) {
            r.Rect = t.Detect.Body.Rect;
        }
    }

private:
    shared_ptr<TrailWorker> trailWorker;
    shared_ptr<RecWorker> recWorker;
};

typedef Singleton<SeemideoAlgo> SeemideoAlgoSingleton;

int32_t Algo_Init(const char *cfg) {
    return SeemideoAlgoSingleton::getInstance().Init(cfg);
}

int32_t Algo_Destroy(void) {
    return SeemideoAlgoSingleton::getInstance().Destroy();
}

int32_t Algo_VideoTrailAndRec(int32_t videoChl, uint64_t timeStamp, const uint8_t *rgbImg, uint32_t height, uint32_t width) {
    SeemideoAlgoSingleton::getInstance().TrailAndRec(videoChl, timeStamp, rgbImg, height, width);
}
