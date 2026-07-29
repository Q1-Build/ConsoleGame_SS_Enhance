#pragma once

#include "Core/IRandomProvider.h"

#include <random>

namespace ss
{
    /// 메르센 트위스터를 사용하는 기본 난수 생성기다.
    class RandomProvider final : public IRandomProvider
    {
    public:
        RandomProvider();

        [[nodiscard]] float NextFloat(float minValue, float maxValue) override;

    private:
        // 모든 난수 요청이 공유하는 엔진 상태다.
        std::mt19937 engine_;
    };
}
