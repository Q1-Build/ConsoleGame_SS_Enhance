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
          bossHealth_(boss.maxHealth)
    {
        assert(swordLevel >= 0);
        assert(boss.maxHealth > 0);
        assert(!boss.attackSequence.empty());
        for (const BossAttackStep& step : boss.attackSequence)
        {
            assert(step.delaySeconds > 0.0f);
            assert(step.warningSeconds > 0.0f);
            assert(step.warningSeconds <= step.delaySeconds);
            assert(step.damage >= 0);
        }
        bossAttackTimeLeft_ = GetCurrentStep().delaySeconds;
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
        if (GetCurrentTelegraph() == AttackTelegraph::Distorted &&
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
            bossAttackTimeLeft_ <= GetCurrentStep().warningSeconds;
    }

    float BattleSession::GetWarningProgress() const noexcept
    {
        if (!IsAttackWarning())
        {
            return 0.0f;
        }

        const float warningSeconds = GetCurrentStep().warningSeconds;
        // 남은 시간을 진행 방향으로 뒤집어 UI 표식이 항상 왼쪽에서 오른쪽으로 이동하게 한다.
        return std::clamp(
            1.0f - bossAttackTimeLeft_ / warningSeconds,
            0.0f,
            1.0f);
    }

    float BattleSession::GetPerfectGuardStartProgress() const noexcept
    {
        const float warningSeconds = GetCurrentStep().warningSeconds;
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

    AttackTelegraph BattleSession::GetCurrentTelegraph() const noexcept
    {
        return GetCurrentStep().telegraph;
    }

    int BattleSession::GetCurrentAttackStep() const noexcept
    {
        return static_cast<int>(attackStepIndex_);
    }

    int BattleSession::GetAttackStepCount() const noexcept
    {
        return static_cast<int>(boss_.attackSequence.size());
    }

    const BossAttackStep& BattleSession::GetCurrentStep() const noexcept
    {
        assert(!boss_.attackSequence.empty());
        assert(attackStepIndex_ < boss_.attackSequence.size());
        return boss_.attackSequence[attackStepIndex_];
    }

    BossAttackResult BattleSession::ResolveBossAttack()
    {
        BossAttackResult result;
        result.guardQuality = pendingGuard_;
        const BossAttackStep resolvedStep = GetCurrentStep();
        const float overdueSeconds = std::max(0.0f, -bossAttackTimeLeft_);

        // 잔상은 방어 상태를 소비하지만 피해를 주지 않아 다음 진짜 공격을 다시 읽게 한다.
        if (resolvedStep.telegraph == AttackTelegraph::Feint)
        {
            result.wasFeint = true;
        }
        else if (pendingGuard_ == GuardQuality::Perfect)
        {
            result.counterDamage = BattleRules::CalculateCounterDamage(swordLevel_);
            bossHealth_ = std::max(0, bossHealth_ - result.counterDamage);
        }
        else if (pendingGuard_ == GuardQuality::Guarded)
        {
            result.damageTaken = BattleRules::CalculateGuardedDamage(
                resolvedStep.damage);
        }
        else
        {
            result.damageTaken = resolvedStep.damage;
        }

        playerHealth_ = std::max(0, playerHealth_ - result.damageTaken);
        pendingGuard_ = GuardQuality::Miss;
        ++resolvedAttackCount_;

        // 다음 단계의 짧은 간격은 장면에 전달해 연속 공격 경고를 즉시 강조한다.
        AdvanceAttackStep(overdueSeconds);
        result.hasQuickFollowUp =
            !IsComplete() &&
            GetCurrentStep().delaySeconds <= 0.8f;
        return result;
    }

    void BattleSession::AdvanceAttackStep(float overdueSeconds) noexcept
    {
        assert(overdueSeconds >= 0.0f);
        attackStepIndex_ =
            (attackStepIndex_ + 1) % boss_.attackSequence.size();

        // 공격 판정을 지난 프레임의 초과 시간을 다음 단계에 넘겨 프레임별 누적 지연을 막는다.
        bossAttackTimeLeft_ =
            GetCurrentStep().delaySeconds - overdueSeconds;
    }
}
