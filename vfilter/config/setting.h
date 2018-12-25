#pragma once

#include <cstdint>
#include <string>
#include <iostream>

#include "common/helper/singleton.h"

namespace video {
namespace filter {

class Settings {
public:
    //使能高创算法
    bool enableGosunAlgo;
    //使能深瞐算法
    bool enableSeemmoAlgo;
    // 高创算法抽帧间隔
    uint32_t gosunFramePickInterval;
    // 深瞐算法抽帧间隔
    uint32_t seemmoFramePickInterval;
    // 默认缓存的最大帧数(30秒的帧数)
    uint32_t frameCacheMaxNum;
    //目标消失初始计数(当某个目标在超过设置的帧后，还没有检测到，则认为目标已经消失，需要从内存中删掉)
    uint32_t objectAbsentCount;
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
    //通知服务接收地址
    std::string notifyServerHost;
    //使用的bufferedFrame类型: 0(原始), 1(做YUV压缩)，2(做jpeg压缩)
    uint32_t bufferedFrameType;

    Settings() {
        init();
        dump();
    }

private:
    void init() {
        //根据环境变量重新设置值
        enableGosunAlgo = parseEnvBoolValue("ENABLE_GOSUN_ALGO", true);
        enableSeemmoAlgo = parseEnvBoolValue("ENABLE_SEEMMO_ALGO", true);
        gosunFramePickInterval = parseEnvNumValue("GOSUN_FRAME_PICK_INTERVAL", 3);
        seemmoFramePickInterval = parseEnvNumValue("SEEMMO_FRAME_PICK_INTERVAL", 5);
        objectAbsentCount = parseEnvNumValue("OBJECT_ABSENT_COUNT", 20);
        facePictureMinWidth = parseEnvNumValue("FACE_PICTURE_MIN_WIDTH", 50);
        personPictureMinWidth = parseEnvNumValue("PERSON_PICTURE_MIN_WIDTH", 80);
        bikePictureMinWidth = parseEnvNumValue("BIKE_PICTURE_MIN_WIDTH", 80);
        vehiclePictureMinWidth = parseEnvNumValue("VEHICLE_PICTURE_MIN_WIDTH", 80);
        frameCacheMaxNum = parseEnvNumValue("FRAME_CACHE_MAX_NUM", 300);
        notifyServerHost = parseEnvStringValue("NOTIFY_SERVER_HOST", "message-transfer:9091");
        scoreDiff4ReRecognize = parseEnvNumValue("SCORE_DIFF_4_RE_RECOGNIZE", 10);
        bufferedFrameType = parseEnvNumValue("BUFFERED_FRAME_TYPE", 1);
    }

    void dump() {
        std::cout << "Settings -> {"
                  << "enableGosunAlgo: " << enableGosunAlgo << ", "
                  << "enableSeemmoAlgo: " << enableSeemmoAlgo << ", "
                  << "gosunFramePickInterval: " << gosunFramePickInterval << ", "
                  << "seemmoFramePickInterval: " << seemmoFramePickInterval << ", "
                  << "frameCacheMaxNum: " << frameCacheMaxNum << ", "
                  << "objectAbsentCount: " << objectAbsentCount << ", "
                  << "scoreDiff4ReRecognize: " << scoreDiff4ReRecognize << ", "
                  << "bufferedFrameType: " << bufferedFrameType << ", "
                  << "facePictureMinWidth: " << facePictureMinWidth << ", "
                  << "personPictureMinWidth: " << personPictureMinWidth << ", "
                  << "bikePictureMinWidth: " << bikePictureMinWidth << ", "
                  << "vehiclePictureMinWidth: " << vehiclePictureMinWidth << ", "
                  << "notifyServerHost: " << notifyServerHost
                  << "}"
                  << std::endl;
    }


    int32_t parseEnvNumValue(const std::string &env, int32_t defValue) {
        char *e = std::getenv(env.c_str());
        if (e != nullptr) {
            return atoi(e);
        }
        return defValue;
    }

    bool parseEnvBoolValue(const std::string &env, bool defValue) {
        char *e = std::getenv(env.c_str());
        if (e != nullptr) {
            uint32_t f = atoi(e);
            return f > 0;
        }
        return defValue;
    }

    std::string parseEnvStringValue(const std::string &env, const std::string &defValue) {
        char *e = std::getenv(env.c_str());
        if (e != nullptr) {
            return std::string(e);
        }
        return defValue;
    }
};

const Settings & G_CFG() {
    return Singleton<Settings>::getInstance();
}

}
}