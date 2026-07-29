#pragma once

#include "Game/Domain/BossBattle.h"

#include <optional>

namespace ss
{
    /// 보스 처치 수에 따른 강화 상한, 도전 대상과 엔딩 조건을 결정한다.
    class ProgressionRules final
    {
    public:
        /// 처치한 보스 수에 따라 현재 강화 가능한 최대 단계를 반환한다.
        [[nodiscard]] static int GetSwordLevelCap(int bossVictoryCount) noexcept;

        /// 현재 단계에서 도전할 보스가 있으면 반환한다.
        [[nodiscard]] static std::optional<BossType> GetAvailableBoss(
            int swordLevel,
            int bossVictoryCount) noexcept;

        /// 세 보스를 모두 처치해 엔딩 조건을 달성했는지 반환한다.
        [[nodiscard]] static bool IsEndingAchieved(int bossVictoryCount) noexcept;
    };
}
