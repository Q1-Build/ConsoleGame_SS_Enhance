#include "Game/Domain/ProgressionRules.h"

#include <algorithm>

namespace ss
{
    int ProgressionRules::GetSwordLevelCap(int bossVictoryCount) noexcept
    {
        // 각 보스가 다음 네 단계의 강화를 해금하며 최종 상한은 +12다.
        const int safeVictoryCount = std::clamp(bossVictoryCount, 0, 2);
        return 4 + safeVictoryCount * 4;
    }

    std::optional<BossType> ProgressionRules::GetAvailableBoss(
        int swordLevel,
        int bossVictoryCount) noexcept
    {
        // 처치 순서를 진행도와 함께 검사해 이미 쓰러뜨린 보스가 다시 해금되지 않게 한다.
        if (bossVictoryCount == 0 && swordLevel >= 4)
        {
            return BossType::EmberWarden;
        }
        if (bossVictoryCount == 1 && swordLevel >= 8)
        {
            return BossType::StormSentinel;
        }
        if (bossVictoryCount == 2 && swordLevel >= 12)
        {
            return BossType::MemoryDevourer;
        }
        return std::nullopt;
    }

    bool ProgressionRules::IsEndingAchieved(int bossVictoryCount) noexcept
    {
        return bossVictoryCount >= 3;
    }
}
