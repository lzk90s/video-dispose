#pragma once

#include "algo/vendor/seemmo/stub_impl/seemmo_stub.h"
#include "algo/vendor/gosun/stub_impl/gosun_stub.h"

using namespace  std;

namespace algo {

//factory
class AlgoStubFactory {
public:
    static shared_ptr<algo::AlgoStub> NewAlgoStub(const string &vendor) {
        shared_ptr<algo::AlgoStub> stub;
        if (vendor == "gosun") {
            stub.reset(new algo::gosun::GosunAlgoStub());
        } else if (vendor == "seemmo") {
            stub.reset(new algo::seemmo::SeemmoAlgoStub());
        } else {
            stub.reset(new algo::AlgoStub());
        }
        return stub;
    }
};

}