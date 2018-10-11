#pragma once

#include <cstdint>
#include <string>
#include <iostream>

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
    //目标消失初始计数(当某个目标在超过设置的帧后，还没有检测到，则认为目标已经消失，需要从内存中删掉)
    uint32_t objectDisappearCount;
    //有效图片的最小宽度，小于这个宽度的图片，比较模糊，不予显示
    uint32_t validPictureMinWidth;
    //目标评分差大于指定值时，需要重新识别
    uint32_t scoreDiff4ReRecognize;
    //使能高创算法
    bool enableGosunAlgo;
    //使能深瞐算法
    bool enableSeemmoAlgo;
    //通知服务接收地址
    string notifyServerHost;

    Settings() {
        init();
        dump();
    }

private:
    void init() {
        //根据环境变量重新设置值
        framePickInternalMs = parseEnvNumValue("FRAME_PICK_INTERNAL_MS", 100);
        framePickInternalNum = parseEnvNumValue("FRAME_PICK_INTERNAL_NUM", 3);
        objectDisappearCount = parseEnvNumValue("OBJECT_DISAPPEAR_COUNT", 10);
        validPictureMinWidth = parseEnvNumValue("VALID_PICTURE_MIN_WIDTH", 80);
        frameCacheMaxNum = parseEnvNumValue("FRAME_CACHE_MAX_NUM", 20 * (1000 / framePickInternalMs));
        notifyServerHost = parseEnvStringValue("NOTIFY_SERVER_HOST", "message-transfer:9091");
        scoreDiff4ReRecognize = parseEnvNumValue("SCORE_DIFF_4_RE_RECOGNIZE", 10);
        enableGosunAlgo = parseEnvBoolValue("ENABLE_GOSUN_ALGO", true);
        enableSeemmoAlgo = parseEnvBoolValue("ENABLE_SEEMMO_ALGO", true);
    }

    uint32_t parseEnvNumValue(const string &env, uint32_t defValue) {
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

    void dump() {
        cout << "Settings -> {"
             << "framePickInternalMs: " << framePickInternalMs << ", "
             << "framePickInternalNum: " << framePickInternalNum << ", "
             << "frameCacheMaxNum: " << frameCacheMaxNum << ", "
             << "objectDisappearCount: " << objectDisappearCount << ", "
             << "enableGosunAlgo: " << enableGosunAlgo << ", "
             << "enableSeemmoAlgo: " << enableSeemmoAlgo << ", "
             << "scoreDiff4ReRecognize: " << scoreDiff4ReRecognize << ", "
             << "notifyServerHost: " << notifyServerHost << "}"
             << endl;
    }
};

typedef Singleton<Settings> GlobalSettings;

}