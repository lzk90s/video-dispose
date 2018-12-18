#pragma once

#include "algo/vendor/seemmo/stub_impl/algo_stub_impl.h"
#include "algo/vendor/gosun/stub_impl/algo_stub_impl.h"

namespace video {
namespace algo {

//factory
class AlgoStubFactory {
public:
    static std::shared_ptr<algo::AlgoStub> NewAlgoStub(const std::string &vendor) {
        std::shared_ptr<algo::AlgoStub> stub;
        if (vendor == "gosun") {
            stub.reset(new algo::gosun::AlgoStubImpl());
        } else if (vendor == "seemmo") {
            stub.reset(new algo::seemmo::AlgoStubImpl());
        } else {
            stub.reset(new algo::AlgoStub());
        }
        return stub;
    }
};

}
}