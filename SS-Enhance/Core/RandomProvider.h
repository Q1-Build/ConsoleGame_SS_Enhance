#pragma once

#include "Core/IRandomProvider.h"

#include <random>

namespace ss
{
    /// 메르센 트위스터를 사용하는 기본 난수 생성기다.
    class RandomProvider final : public IRandomProvider
    {
    public:
        /// 비결정적 시드로 게임 전체에서 공유할 메르센 트위스터를 초기화한다.
        RandomProvider();

        /// 요청 범위에 균등 분포하는 실수를 반환하며 구체 엔진은 외부에 노출하지 않는다.
        [[nodiscard]] float NextFloat(float minValue, float maxValue) override;

    private:
        // 모든 난수 요청이 공유하는 엔진 상태다.
        std::mt19937 engine_;
    };
}
