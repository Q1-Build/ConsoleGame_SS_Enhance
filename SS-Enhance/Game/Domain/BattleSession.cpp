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
        assert(boss.warningSeconds > 0.0f);
        assert(boss.comboCount > 0);
    }

    std::optional<BossAttackResult> BattleSession::Update(float deltaSeconds)
    {
        assert(deltaSeconds >= 0.0f);
        if (IsComplete())
        {
            return {};
        }

        battleTimeSeconds_ += deltaSeconds;
        attackCooldown_ = std::max(0.0f, attackCooldown_ - deltaSeconds);
        guardCooldown_ = std::max(0.0f, guardCooldown_ - deltaSeconds);

        float markerWave = std::sin(battleTimeSeconds_ * 3.4f);
        if (boss_.pattern == BossPattern::MemoryDistortion &&
            IsAttackWarning())
        {
            // 최종 보스는 예고 중 속도를 높이고 공격마다 방향을 뒤집어 공격 타이밍을 교란한다.
            markerWave = std::sin(battleTimeSeconds_ * 7.2f);
            if (resolvedAttackCount_ % 2 != 0)
            {
                markerWave = -markerWave;
            }
        }
        marker_ = markerWave * 0.5f + 0.5f;

        // 보스 공격은 프레임 단위가 아니라 남은 시간으로 계산해 실행 속도와 무관하게 유지한다.
        bossAttackTimeLeft_ -= deltaSeconds;
        if (bossAttackTimeLeft_ <= 0.0f)
        {
            return ResolveBossAttack();
        }
        return {};
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

    std::optional<GuardQuality> BattleSession::TryGuard()
    {
        if (IsComplete() || guardCooldown_ > 0.0f)
        {
            return std::nullopt;
        }

        guardCooldown_ = 0.16f;
        if (!IsAttackWarning())
        {
            // 예고 밖 입력은 실패 피드백만 남기고 이후 공격을 자동으로 막아 주지 않는다.
            pendingGuard_ = GuardQuality::Miss;
            return pendingGuard_;
        }

        pendingGuard_ = bossAttackTimeLeft_ <= kPerfectGuardWindowSeconds
            ? GuardQuality::Perfect
            : GuardQuality::Guarded;
        return pendingGuard_;
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

    int BattleSession::GetPlayerMaxHealth() const noexcept
    {
        return kPlayerMaxHealth;
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

    bool BattleSession::IsAttackWarning() const noexcept
    {
        return !IsComplete() &&
            bossAttackTimeLeft_ <= GetCurrentWarningSeconds();
    }

    float BattleSession::GetWarningProgress() const noexcept
    {
        if (!IsAttackWarning())
        {
            return 0.0f;
        }

        const float warningSeconds = GetCurrentWarningSeconds();
        // 남은 시간을 진행 방향으로 뒤집어 UI 표식이 항상 왼쪽에서 오른쪽으로 이동하게 한다.
        return std::clamp(
            1.0f - bossAttackTimeLeft_ / warningSeconds,
            0.0f,
            1.0f);
    }

    float BattleSession::GetPerfectGuardStartProgress() const noexcept
    {
        const float warningSeconds = GetCurrentWarningSeconds();
        return std::clamp(
            1.0f - kPerfectGuardWindowSeconds / warningSeconds,
            0.0f,
            1.0f);
    }

    bool BattleSession::IsPerfectGuardWindow() const noexcept
    {
        return IsAttackWarning() &&
            GetWarningProgress() >= GetPerfectGuardStartProgress();
    }

    float BattleSession::GetCurrentWarningSeconds() const noexcept
    {
        if (comboAttacksRemaining_ > 0)
        {
            return std::min(
                boss_.warningSeconds,
                boss_.comboGapSeconds * 0.75f);
        }
        return boss_.warningSeconds;
    }

    BossAttackResult BattleSession::ResolveBossAttack()
    {
        BossAttackResult result;
        result.guardQuality = pendingGuard_;

        // 완벽 방어는 피해를 없애고 반격하며, 일반 방어만 감소된 피해를 적용한다.
        if (pendingGuard_ == GuardQuality::Perfect)
        {
            result.counterDamage = BattleRules::CalculateCounterDamage(swordLevel_);
            bossHealth_ = std::max(0, bossHealth_ - result.counterDamage);
        }
        else if (pendingGuard_ == GuardQuality::Guarded)
        {
            result.damageTaken = BattleRules::CalculateGuardedDamage(
                boss_.attackDamage);
        }
        else
        {
            result.damageTaken = boss_.attackDamage;
        }

        playerHealth_ = std::max(0, playerHealth_ - result.damageTaken);
        pendingGuard_ = GuardQuality::Miss;
        ++resolvedAttackCount_;

        // 폭풍의 파수꾼은 세 타를 짧은 간격으로 이어가고 마지막 타격 뒤 기본 주기로 복귀한다.
        if (comboAttacksRemaining_ > 0)
        {
            --comboAttacksRemaining_;
        }
        else
        {
            comboAttacksRemaining_ = boss_.comboCount - 1;
        }

        if (comboAttacksRemaining_ > 0)
        {
            bossAttackTimeLeft_ = boss_.comboGapSeconds;
            result.hasMoreComboAttacks = !IsComplete();
        }
        else
        {
            bossAttackTimeLeft_ = boss_.attackIntervalSeconds;
        }
        return result;
    }
}
