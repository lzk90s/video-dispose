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
    uint32_t framePickInternalMs = 80;
    // 默认抽帧间隔数目（抽帧检测）
    uint32_t framePickInternalNum = 3;
    // 默认帧识别间隔数（在检测抽帧基础上，再抽帧识别）
    uint32_t frameRecogPickInternalNum = 6;
    // 默认缓存的最大帧数(30秒的帧数)
    uint32_t frameCacheMaxNum = 30 * (1000 / framePickInternalMs);
    //目标消失初始计数(当某个目标在超过设置的帧后，还没有检测到，则认为目标已经消失，需要从内存中删掉)
    uint32_t objectDisappearCount = 8;
    //有效图片的最小宽度，小于这个宽度的图片，比较模糊，不予显示
    uint32_t validPictureMinWidth = 80;
    //使能高创算法
    bool enableGosunAlgo = true;
    //使能深瞐算法
    bool enableSeemmoAlgo = true;
    //通知服务接收地址
    string notifyServerHost = string("message-transfer:9091");

    Settings() {
        init();
        dump();
    }

private:
    void init() {
        //根据环境变量重新设置值
        framePickInternalMs = parseEnvNumValue("FRAME_PICK_INTERNAL_MS", framePickInternalMs);
        framePickInternalNum = parseEnvNumValue("FRAME_PICK_INTERNAL_NUM", framePickInternalNum);
        frameRecogPickInternalNum = parseEnvNumValue("FRAME_RECOG_PICK_INTERNAL_NUM", frameRecogPickInternalNum);
        objectDisappearCount = parseEnvNumValue("OBJECT_DISAPPEAR_COUNT", objectDisappearCount);
        validPictureMinWidth = parseEnvNumValue("VALID_PICTURE_MIN_WIDTH", validPictureMinWidth);
        frameCacheMaxNum = parseEnvNumValue("FRAME_CACHE_MAX_NUM", frameCacheMaxNum);
        notifyServerHost = parseEnvStringValue("NOTIFY_SERVER_HOST", notifyServerHost);
        enableGosunAlgo = parseEnvBoolValue("ENABLE_GOSUN_ALGO", enableGosunAlgo);
        enableSeemmoAlgo = parseEnvBoolValue("ENABLE_SEEMMO_ALGO", enableSeemmoAlgo);
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
             << "frameRecogPickInternalNum: " << frameRecogPickInternalNum << ", "
             << "frameCacheMaxNum: " << frameCacheMaxNum << ", "
             << "objectDisappearCount: " << objectDisappearCount << ", "
             << "enableGosunAlgo: " << enableGosunAlgo << ", "
             << "enableSeemmoAlgo: " << enableSeemmoAlgo << ", "
             << "notifyServerHost: " << notifyServerHost << "}"
             << endl;
    }
};

typedef Singleton<Settings> GlobalSettings;

}