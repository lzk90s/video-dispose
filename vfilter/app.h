#pragma once

#include <string>
#include <stdlib.h>

#include "common/helper/singleton.h"

using namespace std;

namespace vf {

class App {
public:
    const char *VFILTER_HOME_ENV = "VFILTER_HOME";

public:
    App() {
        char *appHome = ::getenv(VFILTER_HOME_ENV);
        if (nullptr == appHome) {
            throw runtime_error(string(VFILTER_HOME_ENV) + " is null, please set it at first");
        }
    }
    ~App() {}

    string GetAppHome() {
        return appHome;
    }

private:
    string appHome;
};

typedef Singleton<App> ThisApp;

}