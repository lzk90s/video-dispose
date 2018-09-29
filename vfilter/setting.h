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
    // Ĭ�ϳ�֡ʱ���������룩
    const static uint32_t FRAME_PICK_INTERNAL_MS = 100;
    // Ĭ�ϳ�֡�����Ŀ����֡��⣩
    const static uint32_t FRAME_PICK_INTERNAL_NUM = 3;
    // Ĭ��֡ʶ���������ڼ���֡�����ϣ��ٳ�֡ʶ��
    const static uint32_t FRAME_RECOG_PICK_INTERNAL_NUM = 6;
    // Ĭ�ϻ�������֡��(30���֡��)
    const static uint32_t FRAME_CACHE_MAX_NUM = 10 * (1000 / FRAME_PICK_INTERNAL_MS);
    //Ŀ����ʧ��ʼ����(��ĳ��Ŀ���ڳ������õ�֡�󣬻�û�м�⵽������ΪĿ���Ѿ���ʧ����Ҫ���ڴ���ɾ��)
    const static uint32_t OBJECT_DISAPPEAR_COUNT = 10;
};

typedef Singleton<Settings> GlobalSettings;

}