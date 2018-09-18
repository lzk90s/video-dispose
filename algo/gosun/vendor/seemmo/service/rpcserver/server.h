#pragma once

#include <sstream>
#include <cstdint>

using namespace std;

namespace algo {
namespace seemmo {

class RpcServer {
public:
    virtual void Run(const string &serverAddress, const string &algoCfgPath) {};
};

RpcServer *NewRpcServer();

}
}
