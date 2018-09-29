#pragma once

#include <cstdint>

#include "common/helper/singleton.h"

namespace vf {

class Settings {
public:
    uint32_t framePickInternalMs = FRAME_PICK_INTERNAL_MS;
    uint32_t framePickInternalNum = FRAME_PICK_INTERNAL_NUM;
    uint32_t frameRecogPickInternalNum = FRAME_RECOG_PICK_INTERNAL_NUM;
    uint32_t frameCacheMaxNum = FRAME_CACHE_MAX_NUM;
    uint32_t objectDisappearCount = OBJECT_DISAPPEAR_COUNT;

private:
    // 默认抽帧时间间隔（毫秒）
    const static uint32_t FRAME_PICK_INTERNAL_MS = 100;
    // 默认抽帧间隔数目（抽帧检测）
    const static uint32_t FRAME_PICK_INTERNAL_NUM = 3;
    // 默认帧识别间隔数（在检测抽帧基础上，再抽帧识别）
    const static uint32_t FRAME_RECOG_PICK_INTERNAL_NUM = 6;
    // 默认缓存的最大帧数(30秒的帧数)
    const static uint32_t FRAME_CACHE_MAX_NUM = 10 * (1000 / FRAME_PICK_INTERNAL_MS);
    //目标消失初始计数(当某个目标在超过设置的帧后，还没有检测到，则认为目标已经消失，需要从内存中删掉)
    const static uint32_t OBJECT_DISAPPEAR_COUNT = 10;
};

typedef Singleton<Settings> GlobalSettings;

}