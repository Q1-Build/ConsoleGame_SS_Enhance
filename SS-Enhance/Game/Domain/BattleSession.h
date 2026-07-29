#pragma once

#include "Game/Domain/BossBattle.h"

#include <cstddef>
#include <optional>

namespace ss
{
    /// 방어 입력이 보스 공격 예고와 얼마나 정확히 맞았는지 나타낸다.
    enum class GuardQuality
    {
        Miss,
        Guarded,
        Perfect
    };

    /// 실제로 실행된 한 번의 보스 공격과 방어 결과를 장면에 전달한다.
    struct BossAttackResult
    {
        // 피해와 반격은 동시에 발생하지 않으며 완벽 방어일 때만 반격 피해가 기록된다.
        int damageTaken = 0;
        GuardQuality guardQuality = GuardQuality::Miss;
        int counterDamage = 0;
        bool wasFeint = false;
        bool hasQuickFollowUp = false;
    };

    /// 한 번의 보스 전투에서 체력, 공격 주기와 타이밍 표시를 관리한다.
    class BattleSession final
    {
    public:
        /// 전투 시작 시 보스 수치와 검 단계를 복사해 세션 도중 변하지 않게 고정한다.
        BattleSession(BossDefinition boss, int swordLevel);

        /// 경과 시간만큼 전투를 갱신하고, 보스가 공격한 프레임에만 결과를 반환한다.
        [[nodiscard]] std::optional<BossAttackResult> Update(float deltaSeconds);

        /// 플레이어 공격이 가능하면 실제 피해량을 반환한다.
        [[nodiscard]] std::optional<int> TryAttack();

        /// 공격 예고 중 방어를 시도하고 입력 정확도를 반환한다.
        [[nodiscard]] std::optional<GuardQuality> TryGuard();

        /// 승패가 확정되어 더 이상 입력을 받을 수 없는 상태인지 반환한다.
        [[nodiscard]] bool IsComplete() const noexcept;

        /// 보스 체력만 먼저 0이 된 경우에만 승리로 판정한다.
        [[nodiscard]] bool IsPlayerVictorious() const noexcept;
        [[nodiscard]] int GetPlayerHealth() const noexcept;
        [[nodiscard]] int GetPlayerMaxHealth() const noexcept;
        [[nodiscard]] int GetBossHealth() const noexcept;
        [[nodiscard]] int GetBossMaxHealth() const noexcept;
        [[nodiscard]] float GetMarker() const noexcept;

        /// 현재 보스 공격이 방어 입력을 받을 수 있는 예고 구간인지 반환한다.
        [[nodiscard]] bool IsAttackWarning() const noexcept;

        /// 현재 예고의 진행도를 시작 0에서 공격 직전 1 범위로 반환한다.
        [[nodiscard]] float GetWarningProgress() const noexcept;

        /// 현재 공격 예고에서 완벽 방어 구간이 시작되는 0~1 위치를 반환한다.
        [[nodiscard]] float GetPerfectGuardStartProgress() const noexcept;
        [[nodiscard]] bool IsPerfectGuardWindow() const noexcept;

        /// 현재 예고의 종류를 반환해 장면이 정직한 공격, 교란과 잔상을 구분하게 한다.
        [[nodiscard]] AttackTelegraph GetCurrentTelegraph() const noexcept;

        [[nodiscard]] int GetCurrentAttackStep() const noexcept;
        [[nodiscard]] int GetAttackStepCount() const noexcept;

    private:
        // 플레이어 기본 체력과 완벽 방어 판정 폭은 모든 보스가 공유하는 전투 규칙이다.
        static constexpr int kPlayerMaxHealth = 100;
        static constexpr float kPerfectGuardWindowSeconds = 0.18f;

        [[nodiscard]] const BossAttackStep& GetCurrentStep() const noexcept;
        [[nodiscard]] BossAttackResult ResolveBossAttack();
        void AdvanceAttackStep(float overdueSeconds) noexcept;

        // 전투 진입 시 고정되는 규칙 값과 승패를 결정하는 체력 상태다.
        BossDefinition boss_;
        int swordLevel_ = 0;
        int playerHealth_ = kPlayerMaxHealth;
        int bossHealth_ = 0;

        // 초 단위 타이머와 0~1 공격 표식을 프레임마다 갱신한다.
        float battleTimeSeconds_ = 0.0f;
        float bossAttackTimeLeft_ = 0.0f;
        float attackCooldown_ = 0.0f;
        float guardCooldown_ = 0.0f;
        float marker_ = 0.0f;

        // 데이터 시퀀스 위치와 다음 보스 공격에 적용할 방어 상태다.
        std::size_t attackStepIndex_ = 0;
        int resolvedAttackCount_ = 0;
        GuardQuality pendingGuard_ = GuardQuality::Miss;
    };
}
