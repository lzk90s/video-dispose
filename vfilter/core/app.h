#pragma once

#include <string>

#include "common/helper/logger.h"
#include "common/helper/singleton.h"

#include "vfilter/config/setting.h"

using namespace std;

namespace vf {

class App {
public:
    App() {
        LOG_INFO("------------------------START------------------------");
        init();
    }
    ~App() {}

private:
    void init() {
        vf::G_CFG();
    }
};

typedef Singleton<App> ThisApp;

}