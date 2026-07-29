#include "Game/Domain/BattleSession.h"

#include "Game/Domain/BattleRules.h"

#include <algorithm>
#include <cassert>
#include <cmath>

namespace ss
{
    BattleSession::BattleSession(BossDefinition boss, int swordLevel)
        : boss_(boss),
          swordLevel_(swordLevel),
          bossHealth_(boss.maxHealth),
          bossAttackTimeLeft_(boss.attackIntervalSeconds)
    {
        assert(swordLevel >= 0);
        assert(boss.maxHealth > 0);
        assert(boss.attackIntervalSeconds > 0.0f);
    }

    void BattleSession::Update(float deltaSeconds)
    {
        assert(deltaSeconds >= 0.0f);
        if (IsComplete())
        {
            return;
        }

        battleTimeSeconds_ += deltaSeconds;
        attackCooldown_ = std::max(0.0f, attackCooldown_ - deltaSeconds);

        // 전투 내부 시간으로 왕복 위치를 만들어 외부 장면 시간에 전투 결과가 좌우되지 않게 한다.
        marker_ = std::sin(battleTimeSeconds_ * 3.4f) * 0.5f + 0.5f;

        // 보스 공격은 프레임 단위가 아니라 남은 시간으로 계산해 실행 속도와 무관하게 유지한다.
        bossAttackTimeLeft_ -= deltaSeconds;
        if (bossAttackTimeLeft_ <= 0.0f)
        {
            playerHealth_ = std::max(0, playerHealth_ - boss_.attackDamage);
            bossAttackTimeLeft_ += boss_.attackIntervalSeconds;
        }
    }

    std::optional<int> BattleSession::TryAttack()
    {
        if (IsComplete() || attackCooldown_ > 0.0f)
        {
            return std::nullopt;
        }

        const float timingAccuracy = 1.0f - std::abs(marker_ - 0.5f) * 2.0f;

        // 세션은 입력 정확도만 구하고 검 단계별 피해 공식은 BattleRules에 위임한다.
        const int damage = BattleRules::CalculatePlayerDamage(
            swordLevel_,
            timingAccuracy);
        bossHealth_ = std::max(0, bossHealth_ - damage);
        attackCooldown_ = 0.32f;
        return damage;
    }

    bool BattleSession::IsComplete() const noexcept
    {
        return playerHealth_ <= 0 || bossHealth_ <= 0;
    }

    bool BattleSession::IsPlayerVictorious() const noexcept
    {
        return bossHealth_ <= 0 && playerHealth_ > 0;
    }

    int BattleSession::GetPlayerHealth() const noexcept
    {
        return playerHealth_;
    }

    int BattleSession::GetBossHealth() const noexcept
    {
        return bossHealth_;
    }

    int BattleSession::GetBossMaxHealth() const noexcept
    {
        return boss_.maxHealth;
    }

    float BattleSession::GetMarker() const noexcept
    {
        return marker_;
    }

    float BattleSession::GetAttackCooldown() const noexcept
    {
        return attackCooldown_;
    }
}
