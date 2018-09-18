#pragma once

#include <fstream>
#include <iostream>

#include <sstream>
#include "common/helper/singleton.h"
#include "json/json.hpp"
#include "algo/seemmo/vendor/algo.h"

using json = nlohmann::json;
using namespace std;

typedef struct SeemideoCfg {
    string baseDir;
    int32_t imgCoreNum;
    int32_t videoCoreNum;
    string authServer;
    int32_t authType;
    int32_t devId;
} SEEMIDEO_CFG;

typedef struct GosunCfg {

} GOSUN_CFG;

class AlgoSetting {
public:
    SEEMIDEO_CFG seemideoCfg;
    GOSUN_CFG gosunCfg;

public:
    int32_t Init(const string &f) {
        int32_t ret = 0;
        ret = parseSeemideo(f, seemideoCfg);
        if (E_OK != ret) {
            return ret;
        }

        ret = parseGosun(f, gosunCfg);
        if (E_OK != ret) {
            return ret;
        }

        return E_OK;
    }

private:
    int32_t parseSeemideo(const string &f, SEEMIDEO_CFG &cfg) {
        ifstream fin(f);
        if (!fin.is_open()) {
            cout << "Failed to open config file" << f.c_str() << endl;
            return E_FILEOPEN;
        }

        stringstream stm;
        string s;
        while (fin >> s) {
            stm << s;
        }

        const char *SEEMIDEO_CFG_KEY = "seemideo";

        auto j = json::parse(stm.str());
        if (j.empty() || j[SEEMIDEO_CFG_KEY].empty()) {
            cout << "json is empty" << endl;
            return E_INVALID_CFG;
        }

        auto o = j[SEEMIDEO_CFG_KEY];
        cfg.baseDir = o["base_dir"].get<std::string>();
        cfg.imgCoreNum = o["img_core_num"].get<int>();
        cfg.videoCoreNum = o["video_core_num"].get<int>();
        cfg.authServer = o["auth_server"].get<std::string>();
        cfg.authType = o["auth_type"].get<int>();
        cfg.devId = o["dev_id"].get<int>();
    }

    int32_t parseGosun(const string &f, GOSUN_CFG &cfg) {
        return E_OK;
    }

};

typedef Singleton<AlgoSetting> AlgoSettingSingleton;

#define ALGO_SETTING() AlgoSettingSingleton::getInstance()
