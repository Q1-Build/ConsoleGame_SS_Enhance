#include "Game/Domain/BattleRules.h"

#include <algorithm>

namespace ss
{
    BossDefinition BattleRules::GetBossDefinition(BossType bossType) noexcept
    {
        // 보스 수치는 첫 수직 슬라이스의 진행 시간을 짧게 유지하도록 보수적으로 설정한다.
        switch (bossType)
        {
        case BossType::EmberWarden:
            return {bossType, 90, 9, 2.4f, 700, 1};
        case BossType::StormSentinel:
            return {bossType, 170, 13, 2.1f, 1200, 2};
        case BossType::MemoryDevourer:
            return {bossType, 280, 17, 1.8f, 2200, 3};
        }
        return {};
    }

    int BattleRules::CalculatePlayerDamage(
        int swordLevel,
        float timingAccuracy) noexcept
    {
        const float safeAccuracy = std::clamp(timingAccuracy, 0.0f, 1.0f);
        const int baseDamage = 8 + std::max(0, swordLevel) * 2;

        // 빗맞혀도 최소 피해를 주되 중앙 타격은 기본 피해의 1.5배까지 보상한다.
        return static_cast<int>(
            static_cast<float>(baseDamage) * (0.5f + safeAccuracy));
    }
}
