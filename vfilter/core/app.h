#pragma once

#include <string>

#include "common/helper/singleton.h"

using namespace std;

namespace vf {

class App {
public:
    const char *VFILTER_HOME_ENV = "VFILTER_HOME";

public:
    App() {
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