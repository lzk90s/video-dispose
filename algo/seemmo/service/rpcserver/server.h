#pragma once

#include <sstream>
#include <cstdint>

using namespace std;

class RpcServer {
public:
    virtual void Run(const string &serverAddress, const string &algoCfgPath) {};
};

RpcServer *NewRpcServer();
