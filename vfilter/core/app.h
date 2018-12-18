#pragma once

#include <string>

#include "common/helper/logger.h"
#include "common/helper/singleton.h"

#include "vfilter/config/setting.h"

namespace video {
namespace filter {

class App {
public:
    App() {
        LOG_INFO("------------------------START------------------------");
    }
    ~App() {}
};

typedef Singleton<App> ThisApp;

}
}