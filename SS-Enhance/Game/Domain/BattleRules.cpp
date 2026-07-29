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
            return {
                bossType,
                BossPattern::HeavyStrike,
                180,
                20,
                3.2f,
                1.15f,
                1,
                0.0f,
                700,
                1};
        case BossType::StormSentinel:
            return {
                bossType,
                BossPattern::TripleCombo,
                300,
                8,
                2.5f,
                0.65f,
                3,
                0.42f,
                1200,
                2};
        case BossType::MemoryDevourer:
            return {
                bossType,
                BossPattern::MemoryDistortion,
                500,
                15,
                2.2f,
                0.85f,
                1,
                0.0f,
                2200,
                3};
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

    int BattleRules::CalculateCounterDamage(int swordLevel) noexcept
    {
        // 완벽 방어는 완벽 공격 피해의 절반을 돌려줘 방어 선택에도 공격 가치를 부여한다.
        return std::max(1, CalculatePlayerDamage(swordLevel, 1.0f) / 2);
    }

    int BattleRules::CalculateGuardedDamage(int attackDamage) noexcept
    {
        // 일반 방어는 위협을 남기면서도 연속 공격을 버틸 수 있도록 원래 피해의 35%만 적용한다.
        return std::max(1, std::max(0, attackDamage) * 35 / 100);
    }
}
