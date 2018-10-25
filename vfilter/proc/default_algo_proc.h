#pragma once

#include "vfilter/proc/abstract_algo_proc.h"

using namespace std;
using namespace algo;

namespace vf {

class DefaultAlgoProcessor : public AbstractAlgoProcessor {
public:
    DefaultAlgoProcessor(VSink &vsink, AlgoStub *stub = NewAlgoStub(GlobalSettings::getInstance().enableSeemmoAlgo,
                         false)) :
        AbstractAlgoProcessor(vsink, *stub) {
        this->stub_ = stub;
    }

    ~DefaultAlgoProcessor() {
        FreeAlgoStub(stub_);
    }

private:
    AlgoStub * stub_;
};

}