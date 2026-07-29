#pragma once

#include "Game/Domain/BossBattle.h"

namespace ss
{
    /// 보스별 전투 수치와 타이밍 공격 피해를 계산하는 순수 규칙 서비스다.
    class BattleRules final
    {
    public:
        /// 보스 종류에 대응하는 체력, 공격 패턴과 보상 묶음을 반환한다.
        [[nodiscard]] static BossDefinition GetBossDefinition(BossType bossType) noexcept;

        /// 검 단계와 0~1 타이밍 정확도를 반영한 플레이어 공격 피해를 계산한다.
        [[nodiscard]] static int CalculatePlayerDamage(
            int swordLevel,
            float timingAccuracy) noexcept;

        /// 완벽 방어 시 검 단계에 비례해 되돌려 줄 반격 피해를 계산한다.
        [[nodiscard]] static int CalculateCounterDamage(int swordLevel) noexcept;

        /// 일반 방어에 성공했을 때 실제로 받을 피해를 계산한다.
        [[nodiscard]] static int CalculateGuardedDamage(int attackDamage) noexcept;
    };
}
