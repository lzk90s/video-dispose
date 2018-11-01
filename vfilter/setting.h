#pragma once

#include <cstdint>
#include <string>
#include <iostream>
#include <limits>

#include "common/helper/singleton.h"

using namespace std;

namespace vf {

class Settings {
public:
    // 默认抽帧时间间隔（毫秒）
    uint32_t framePickInternalMs;
    // 默认抽帧间隔数目（抽帧检测）
    uint32_t framePickInternalNum;
    // 默认缓存的最大帧数(30秒的帧数)
    uint32_t frameCacheMaxNum;
    //是否压缩缓存的帧
    bool compressFrameCache;
    //目标消失初始计数(当某个目标在超过设置的帧后，还没有检测到，则认为目标已经消失，需要从内存中删掉)
    uint32_t objectDisappearCount;
    //人脸图片最小宽度
    uint32_t facePictureMinWidth;
    //行人图片最小宽度
    uint32_t personPictureMinWidth;
    //非机动车图片最小宽度
    uint32_t bikePictureMinWidth;
    //机动车图片最小宽度
    uint32_t vehiclePictureMinWidth;
    //目标评分差大于指定值时，需要重新识别
    uint32_t scoreDiff4ReRecognize;
    //使能高创算法
    bool enableGosunAlgo;
    //使能深瞐算法
    bool enableSeemmoAlgo;
    //通知服务接收地址
    string notifyServerHost;
    //使用的bufferedFrame类型: 0(原始), 1(做YUV压缩)，2(做jpeg压缩)
    uint32_t bufferedFrameType;

    Settings() {
        init();
        dump();
    }

private:
    void init() {
        //根据环境变量重新设置值
        framePickInternalMs = parseEnvNumValue("FRAME_PICK_INTERNAL_MS", 100);
        framePickInternalNum = parseEnvNumValue("FRAME_PICK_INTERNAL_NUM", 5);
        objectDisappearCount = parseEnvNumValue("OBJECT_DISAPPEAR_COUNT", 10);
        facePictureMinWidth = parseEnvNumValue("FACE_PICTURE_MIN_WIDTH", 80);
        personPictureMinWidth = parseEnvNumValue("PERSON_PICTURE_MIN_WIDTH", 80);
        bikePictureMinWidth = parseEnvNumValue("BIKE_PICTURE_MIN_WIDTH", 80);
        vehiclePictureMinWidth = parseEnvNumValue("VEHICLE_PICTURE_MIN_WIDTH", 80);
        frameCacheMaxNum = parseEnvNumValue("FRAME_CACHE_MAX_NUM", 300);
        compressFrameCache = parseEnvBoolValue("COMPRESS_FRAME_CACHE", true);
        notifyServerHost = parseEnvStringValue("NOTIFY_SERVER_HOST", "message-transfer:9091");
        scoreDiff4ReRecognize = parseEnvNumValue("SCORE_DIFF_4_RE_RECOGNIZE", 10);
        enableGosunAlgo = parseEnvBoolValue("ENABLE_GOSUN_ALGO", true);
        enableSeemmoAlgo = parseEnvBoolValue("ENABLE_SEEMMO_ALGO", true);
        bufferedFrameType = parseEnvNumValue("BUFFERED_FRAME_TYPE", 1);
    }

    void dump() {
        cout << "Settings -> {"
             << "framePickInternalMs: " << framePickInternalMs << ", "
             << "framePickInternalNum: " << framePickInternalNum << ", "
             << "frameCacheMaxNum: " << frameCacheMaxNum << ", "
             << "compressFrameCache: " << compressFrameCache << ", "
             << "objectDisappearCount: " << objectDisappearCount << ", "
             << "enableGosunAlgo: " << enableGosunAlgo << ", "
             << "enableSeemmoAlgo: " << enableSeemmoAlgo << ", "
             << "scoreDiff4ReRecognize: " << scoreDiff4ReRecognize << ", "
             << "bufferedFrameType: " << bufferedFrameType << ", "
             << "facePictureMinWidth: " << facePictureMinWidth << ", "
             << "personPictureMinWidth: " << personPictureMinWidth << ", "
             << "bikePictureMinWidth: " << bikePictureMinWidth << ", "
             << "vehiclePictureMinWidth: " << vehiclePictureMinWidth << ", "
             << "notifyServerHost: " << notifyServerHost
             << "}"
             << endl;
    }


    int32_t parseEnvNumValue(const string &env, int32_t defValue) {
        char *e = std::getenv(env.c_str());
        if (e != nullptr) {
            return atoi(e);
        }
        return defValue;
    }

    bool parseEnvBoolValue(const string &env, bool defValue) {
        char *e = std::getenv(env.c_str());
        if (e != nullptr) {
            uint32_t f = atoi(e);
            return f > 0;
        }
        return defValue;
    }

    string parseEnvStringValue(const string &env, const string &defValue) {
        char *e = std::getenv(env.c_str());
        if (e != nullptr) {
            return string(e);
        }
        return defValue;
    }
};

typedef Singleton<Settings> GlobalSettings;

}