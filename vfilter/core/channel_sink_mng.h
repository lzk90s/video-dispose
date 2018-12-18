#pragma once

#include <memory>
#include "vfilter/core/channel_sink.h"

namespace video {
namespace filter {

//channel sink manager
//暂不加锁，无需求
class ChannelSinkManager {
public:
    std::map<uint32_t, std::shared_ptr<ChannelSink>> sinks;

    ~ChannelSinkManager() {
        sinks.clear();
    }
};

ChannelSinkManager& CSMS() {
    typedef Singleton<ChannelSinkManager> ChannelSinkManagerSingleton;
    return ChannelSinkManagerSingleton::getInstance();
}

}
}