#include "Core/RandomProvider.h"

namespace ss
{
    RandomProvider::RandomProvider()
        : engine_(std::random_device{}())
    {
    }

    float RandomProvider::NextFloat(float minValue, float maxValue)
    {
        std::uniform_real_distribution<float> distribution(minValue, maxValue);
        return distribution(engine_);
    }
}
