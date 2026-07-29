#include "Core/RandomProvider.h"

namespace ss
{
    RandomProvider::RandomProvider()
        // 실행할 때마다 다른 강화 결과와 파티클 패턴이 나오도록 시스템 엔트로피로 시드한다.
        : engine_(std::random_device{}())
    {
    }

    float RandomProvider::NextFloat(float minValue, float maxValue)
    {
        std::uniform_real_distribution<float> distribution(minValue, maxValue);
        return distribution(engine_);
    }
}
