#include "algo/stub/algo_stub.h"
#include "algo/vendor/seemmo/stub_impl/seemmo_stub.h"

namespace algo {


AlgoStub* AlgoStubFactory::CreateStub(const string &vendor) {
    if (vendor == "seemmo") {
        return new algo::seemmo::SeemmoAlgoStub();
    } else if (vendor == "gosun") {
        return nullptr;
    } else {
        throw runtime_error("invalid vendor " + vendor);
    }
}

void AlgoStubFactory::FreeStub(AlgoStub *&stub) {
    if (nullptr == stub) {
        return;
    }

    if (stub->GetVendor() == "seemmo") {
        delete stub;
        stub = nullptr;
    } else if (stub->GetVendor() == "gosun") {
        //
    } else {
        throw runtime_error("invalid vendor " + stub->GetVendor());
    }
}

}