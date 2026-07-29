#pragma once

#include "Game/Domain/Sword.h"

namespace ss
{
    /// 플레이어의 검, 재화, 강화 기록을 하나의 유효한 진행 상태로 관리한다.
    class PlayerProgress final
    {
    public:
        [[nodiscard]] const Sword& GetSword() const noexcept;
        [[nodiscard]] int GetGold() const noexcept;
        [[nodiscard]] int GetFragments() const noexcept;
        [[nodiscard]] int GetAttemptCount() const noexcept;
        [[nodiscard]] int GetSuccessCount() const noexcept;
        [[nodiscard]] int GetBossVictoryCount() const noexcept;
        [[nodiscard]] bool CanAfford(int amount) const noexcept;

        /// 골드가 충분하면 차감하고 성공 여부를 반환한다.
        [[nodiscard]] bool SpendGold(int amount);
        void GrantGold(int amount);

        /// 기억 조각이 있으면 하나를 소비하고 성공 여부를 반환한다.
        [[nodiscard]] bool ConsumeFragment();

        /// 전투 보상으로 받은 기억 조각을 음수가 되지 않게 누적한다.
        void GrantFragments(int amount);
        void RecordAttempt() noexcept;
        void RecordSuccess() noexcept;

        /// 현재 강화 구간의 보스 처치를 기록해 다음 구간을 해금한다.
        void RecordBossVictory() noexcept;
        void EnhanceSword(int levelCount);
        void DowngradeSword();

    private:
        // 모든 재화와 누적 기록은 음수가 되지 않는다.
        Sword sword_;
        int gold_ = 1800;
        int fragments_ = 2;
        int attemptCount_ = 0;
        int successCount_ = 0;
        int bossVictoryCount_ = 0;
    };
}
