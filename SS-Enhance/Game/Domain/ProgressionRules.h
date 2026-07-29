#pragma once

#include "Game/Domain/BossBattle.h"

#include <optional>

namespace ss
{
    /// 보스 처치 수에 따른 강화 상한, 도전 대상과 엔딩 조건을 결정한다.
    class ProgressionRules final
    {
    public:
        [[nodiscard]] static int GetSwordLevelCap(int bossVictoryCount) noexcept;

        /// 현재 단계에서 도전할 보스가 있으면 반환한다.
        [[nodiscard]] static std::optional<BossType> GetAvailableBoss(
            int swordLevel,
            int bossVictoryCount) noexcept;

        [[nodiscard]] static bool IsEndingAchieved(int bossVictoryCount) noexcept;
    };
}
