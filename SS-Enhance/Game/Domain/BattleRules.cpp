#include "Game/Domain/BattleRules.h"

#include <algorithm>

namespace ss
{
    BossDefinition BattleRules::GetBossDefinition(BossType bossType) noexcept
    {
        // 보스별 수치와 시퀀스를 한 표처럼 모아 패턴과 보상 밸런스를 함께 조정한다.
        switch (bossType)
        {
        case BossType::EmberWarden:
            return {
                bossType,
                180,
                {
                    {3.4f, 1.35f, 24, AttackTelegraph::Honest}
                },
                {900, 0},
                {300, 2}};
        case BossType::StormSentinel:
            return {
                bossType,
                300,
                {
                    {2.5f, 0.75f, 8, AttackTelegraph::Honest},
                    {0.62f, 0.42f, 8, AttackTelegraph::Honest},
                    {0.38f, 0.28f, 10, AttackTelegraph::Honest}
                },
                {1500, 0},
                {600, 3}};
        case BossType::MemoryDevourer:
            return {
                bossType,
                500,
                {
                    {2.2f, 0.85f, 15, AttackTelegraph::Distorted},
                    {1.7f, 0.75f, 0, AttackTelegraph::Feint},
                    {0.55f, 0.36f, 18, AttackTelegraph::Honest}
                },
                {2600, 0},
                {1000, 4}};
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
