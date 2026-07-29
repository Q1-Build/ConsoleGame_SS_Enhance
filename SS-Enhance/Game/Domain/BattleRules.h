#pragma once

#include "Game/Domain/BossBattle.h"

namespace ss
{
    /// 보스별 전투 수치와 타이밍 공격 피해를 계산하는 순수 규칙 서비스다.
    class BattleRules final
    {
    public:
        [[nodiscard]] static BossDefinition GetBossDefinition(BossType bossType) noexcept;

        [[nodiscard]] static int CalculatePlayerDamage(
            int swordLevel,
            float timingAccuracy) noexcept;
    };
}
