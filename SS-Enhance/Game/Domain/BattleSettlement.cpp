#include "Game/Domain/BattleSettlement.h"

#include "Game/Domain/PlayerProgress.h"

#include <cassert>

namespace ss
{
    bool BattleSettlement::ApplyVictory(
        PlayerProgress& progress,
        BattleReward reward)
    {
        assert(reward.gold >= 0);
        assert(reward.fragments >= 0);
        if (isApplied_)
        {
            return false;
        }

        // 재화와 진행도 해금을 같은 확정 지점에서 반영해 일부 보상만 지급되는 상태를 막는다.
        progress.GrantGold(reward.gold);
        progress.GrantFragments(reward.fragments);
        progress.RecordBossVictory();
        isApplied_ = true;
        return true;
    }

    bool BattleSettlement::ApplyDefeat(PlayerProgress& progress)
    {
        if (isApplied_)
        {
            return false;
        }

        progress.DowngradeSword();
        isApplied_ = true;
        return true;
    }

    bool BattleSettlement::IsApplied() const noexcept
    {
        return isApplied_;
    }
}
