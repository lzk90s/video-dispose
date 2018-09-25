#pragma once

#include <memory>

#include "algo/stub/algo_stub.h"

using namespace  std;

namespace algo {
namespace seemmo {

algo::AlgoStub* NewAlgoStub();

void FreeAlgoStub(algo::AlgoStub *&stub);

}
}
