#include "Game/Domain/ProgressionRules.h"

#include <algorithm>
#include <array>

namespace ss
{
    namespace
    {
        // 보스 순서와 도전 단계를 같은 인덱스로 관리해 진행 조건을 추가할 때 분기 누락을 막는다.
        constexpr std::array<BossType, 3> kBossOrder =
        {
            BossType::EmberWarden,
            BossType::StormSentinel,
            BossType::MemoryDevourer
        };
        constexpr std::array<int, kBossOrder.size()> kBossMilestones =
        {
            4,
            8,
            12
        };
    }

    int ProgressionRules::GetSwordLevelCap(int bossVictoryCount) noexcept
    {
        const int safeVictoryCount = std::clamp(
            bossVictoryCount,
            0,
            static_cast<int>(kBossMilestones.size()) - 1);
        return kBossMilestones[static_cast<std::size_t>(safeVictoryCount)];
    }

    std::optional<BossType> ProgressionRules::GetAvailableBoss(
        int swordLevel,
        int bossVictoryCount) noexcept
    {
        if (bossVictoryCount < 0 ||
            bossVictoryCount >= static_cast<int>(kBossOrder.size()))
        {
            return std::nullopt;
        }

        const std::size_t bossIndex =
            static_cast<std::size_t>(bossVictoryCount);
        if (swordLevel >= kBossMilestones[bossIndex])
        {
            return kBossOrder[bossIndex];
        }
        return std::nullopt;
    }

    bool ProgressionRules::IsEndingAchieved(int bossVictoryCount) noexcept
    {
        return bossVictoryCount >= static_cast<int>(kBossOrder.size());
    }
}
