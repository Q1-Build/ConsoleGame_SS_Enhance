#pragma once

#include "Game/Domain/BossBattle.h"

namespace ss
{
    class PlayerProgress;

    /// 한 전투의 승리 보상 또는 패배 페널티를 정확히 한 번만 진행도에 반영한다.
    class BattleSettlement final
    {
    public:
        /// 선택한 승리 보상을 지급하고 보스 처치를 기록한다.
        /// 이미 결과가 적용됐다면 상태를 바꾸지 않고 false를 반환한다.
        [[nodiscard]] bool ApplyVictory(
            PlayerProgress& progress,
            BattleReward reward);

        /// 검 강화 단계를 한 단계 낮추며 이미 결과가 적용됐다면 false를 반환한다.
        [[nodiscard]] bool ApplyDefeat(PlayerProgress& progress);

        [[nodiscard]] bool IsApplied() const noexcept;

    private:
        bool isApplied_ = false;
    };
}
