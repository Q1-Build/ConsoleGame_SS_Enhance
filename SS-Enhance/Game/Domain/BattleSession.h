#pragma once

#include "Game/Domain/BossBattle.h"

#include <optional>

namespace ss
{
    /// 한 번의 보스 전투에서 체력, 공격 주기와 타이밍 표시를 관리한다.
    class BattleSession final
    {
    public:
        BattleSession(BossDefinition boss, int swordLevel);

        /// 경과 시간만큼 보스 공격과 타이밍 표시를 갱신한다.
        void Update(float deltaSeconds);

        /// 플레이어 공격이 가능하면 실제 피해량을 반환한다.
        [[nodiscard]] std::optional<int> TryAttack();

        [[nodiscard]] bool IsComplete() const noexcept;
        [[nodiscard]] bool IsPlayerVictorious() const noexcept;
        [[nodiscard]] int GetPlayerHealth() const noexcept;
        [[nodiscard]] int GetBossHealth() const noexcept;
        [[nodiscard]] int GetBossMaxHealth() const noexcept;
        [[nodiscard]] float GetMarker() const noexcept;
        [[nodiscard]] float GetAttackCooldown() const noexcept;

    private:
        BossDefinition boss_;
        int swordLevel_ = 0;
        int playerHealth_ = 100;
        int bossHealth_ = 0;
        float battleTimeSeconds_ = 0.0f;
        float bossAttackTimeLeft_ = 0.0f;
        float attackCooldown_ = 0.0f;
        float marker_ = 0.0f;
    };
}
