#include "MemoryDelayEngine.h"

#include <cassert>
#include <vector>

using echoform::MemoryDelayEngine;
using echoform::MemoryDelayParameters;
using echoform::ScanMode;

namespace {
void testWraparound() {
    MemoryDelayEngine engine;
    engine.prepare(10.0, 16, 1.0);

    MemoryDelayParameters params;
    params.sizeMs = 1000.0f;
    params.scanMode = ScanMode::Manual;
    params.scan = 1.0f;
    engine.setParameters(params);

    std::vector<float> inputL(25, 0.0f);
    std::vector<float> inputR(25, 0.0f);
    for (int i = 0; i < static_cast<int>(inputL.size()); ++i) {
        inputL[i] = static_cast<float>(i + 1);
        inputR[i] = static_cast<float>(i + 1) * 2.0f;
    }

    std::vector<float> outputL(inputL.size(), 0.0f);
    std::vector<float> outputR(inputL.size(), 0.0f);

    const float* inputs[2] = {inputL.data(), inputR.data()};
    float* outputs[2] = {outputL.data(), outputR.data()};

    engine.processBlock(inputs, outputs, static_cast<int>(inputL.size()));

    const int writeIndex = engine.getWriteIndex();
    const int lastIndex = (writeIndex - 1 + engine.getMaxSamples()) % engine.getMaxSamples();

    assert(engine.debugGetMemorySample(0, lastIndex) == inputL.back());
    assert(engine.debugGetMemorySample(1, lastIndex) == inputR.back());
}
} // namespace

int main() {
    testWraparound();
    return 0;
}
