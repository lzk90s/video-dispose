#pragma once

#include <memory>
#include "vfilter/core/channel_sink.h"

using namespace std;

namespace vf {
//channel sink manager
//暂不加锁，无需求
class ChannelSinkManager {
public:
    map<uint32_t, shared_ptr<ChannelSink>> sinks;

    ~ChannelSinkManager() {
        sinks.clear();
    }
};

ChannelSinkManager& CSMS() {
    typedef Singleton<ChannelSinkManager> ChannelSinkManagerSingleton;
    return ChannelSinkManagerSingleton::getInstance();
}

}