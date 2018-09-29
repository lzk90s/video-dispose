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
    uint32_t framePickInternalMs = 100;
    // 默认抽帧间隔数目（抽帧检测）
    uint32_t framePickInternalNum = 3;
    // 默认帧识别间隔数（在检测抽帧基础上，再抽帧识别）
    uint32_t frameRecogPickInternalNum = 6;
    // 默认缓存的最大帧数(30秒的帧数)
    uint32_t frameCacheMaxNum = 30 * (1000 / framePickInternalMs);
    //目标消失初始计数(当某个目标在超过设置的帧后，还没有检测到，则认为目标已经消失，需要从内存中删掉)
    uint32_t objectDisappearCount = 10;
    //通知服务接收地址
    string notifyServerHost = (std::getenv("NOTIFY_SERVER_HOST")) ? string(std::getenv("NOTIFY_SERVER_HOST")) :
                              string("http://message-transfer:9091");

    Settings() {
        dump();
    }

private:
    void dump() {
        cout << "Settings -> {"
             << "framePickInternalMs: " << framePickInternalMs << ", "
             << "framePickInternalNum: " << framePickInternalNum << ", "
             << "frameRecogPickInternalNum: " << frameRecogPickInternalNum << ", "
             << "frameCacheMaxNum: " << frameCacheMaxNum << ", "
             << "objectDisappearCount: " << objectDisappearCount << ", "
             << "notifyServerHost: " << notifyServerHost << "}"
             << endl;
    }
};

typedef Singleton<Settings> GlobalSettings;

}