#pragma once

#include "Game/Domain/Difficulty.h"
#include "Game/Domain/ForgeOutcome.h"

#include <vector>

namespace ss
{
    class PlayerProgress;

    /// 강화 비용, 성공 확률, 실패 패널티를 한곳에서 판정하는 순수 도메인 서비스다.
    class ForgeRules final
    {
    public:
        /// 검 단계의 기본 비용에 선택한 난이도 배율을 적용한다.
        [[nodiscard]] static int CalculateCost(
            int swordLevel,
            Difficulty difficulty) noexcept;

        [[nodiscard]] static float GetBaseChance(int swordLevel) noexcept;
        [[nodiscard]] static float CalculateCraftScore(const std::vector<float>& strikeScores) noexcept;
        [[nodiscard]] static float CalculateFinalChance(int swordLevel, float craftScore) noexcept;

        /// 전달된 난수 값으로 강화 결과를 결정하고 플레이어 진행도에 원자적으로 반영한다.
        [[nodiscard]] ForgeOutcome Resolve(
            PlayerProgress& progress,
            const std::vector<float>& strikeScores,
            float randomRoll) const;

    private:
        [[nodiscard]] static constexpr float Clamp01(float value) noexcept
        {
            return value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
        }
    };
}
