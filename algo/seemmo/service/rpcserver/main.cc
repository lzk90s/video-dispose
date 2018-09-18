#include <iostream>
#include <string>
#include "algo/seemmo/service/rpcserver/server.h"

using namespace  std;

void usage(const char*program) {
    cout << "Usage: " << program << " <server_address> <algo_cfg_path>" << endl;
}

int main(int argc, char**argv) {
    if (argc != 3) {
        usage(argv[0]);
        exit(-1);
    }
    string serverAddress = argv[1];
    string algoCfgPath = argv[2];

    RpcServer *server = NewRpcServer();
    server->Run(serverAddress, algoCfgPath);
    return 0;
}
