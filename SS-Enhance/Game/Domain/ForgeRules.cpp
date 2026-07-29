#include "Game/Domain/ForgeRules.h"

#include "Game/Domain/PlayerProgress.h"

#include <algorithm>
#include <array>
#include <cassert>

namespace ss
{
    int ForgeRules::CalculateCost(int swordLevel, Difficulty difficulty) noexcept
    {
        const int baseCost = 120 + swordLevel * 95 + swordLevel * swordLevel * 12;
        const DifficultyTuning tuning = GetDifficultyTuning(difficulty);

        // 난이도는 성공 확률을 직접 낮추지 않고 준비 비용을 완만하게 조절한다.
        return baseCost * tuning.forgeCostPercent / 100;
    }

    float ForgeRules::GetBaseChance(int swordLevel) noexcept
    {
        // 확률 테이블은 강화 단계별 밸런스를 코드 한곳에서 조정할 수 있게 모아둔다.
        static constexpr std::array<float, 12> kBaseChances =
        {
            0.96f, 0.90f, 0.82f, 0.73f, 0.63f, 0.52f,
            0.41f, 0.31f, 0.22f, 0.14f, 0.08f, 0.04f
        };

        const int safeLevel = std::max(0, std::min(swordLevel, 11));
        return kBaseChances[static_cast<std::size_t>(safeLevel)];
    }

    float ForgeRules::CalculateCraftScore(const std::vector<float>& strikeScores) noexcept
    {
        constexpr int kRequiredStrikeCount = 3;
        float scoreSum = 0.0f;
        const std::size_t scoreCount = std::min(
            strikeScores.size(),
            static_cast<std::size_t>(kRequiredStrikeCount));

        for (std::size_t index = 0; index < scoreCount; ++index)
        {
            scoreSum += Clamp01(strikeScores[index]);
        }

        // 시도하지 못한 타격은 0점으로 계산하므로 항상 3으로 나눈다.
        return scoreSum / static_cast<float>(kRequiredStrikeCount);
    }

    float ForgeRules::CalculateFinalChance(int swordLevel, float craftScore) noexcept
    {
        // 플레이 실력이 기본 확률을 완전히 대체하지 않고 62~110% 범위에서 보정한다.
        const float skillMultiplier = 0.62f + Clamp01(craftScore) * 0.48f;
        const float calculatedChance = GetBaseChance(swordLevel) * skillMultiplier;
        return std::max(0.02f, std::min(calculatedChance, 0.98f));
    }

    ForgeOutcome ForgeRules::Resolve(
        PlayerProgress& progress,
        const std::vector<float>& strikeScores,
        float randomRoll) const
    {
        assert(randomRoll >= 0.0f && randomRoll <= 1.0f);

        ForgeOutcome outcome;
        outcome.previousLevel = progress.GetSword().GetLevel();
        outcome.craftScore = CalculateCraftScore(strikeScores);
        outcome.finalChance = CalculateFinalChance(outcome.previousLevel, outcome.craftScore);
        outcome.wasCritical =
            outcome.craftScore >= 0.94f &&
            randomRoll < outcome.finalChance * 0.20f;
        outcome.succeeded = randomRoll < outcome.finalChance;

        // 판정과 진행도 반영을 한 트랜잭션으로 처리해 화면과 실제 상태가 어긋나지 않게 한다.
        if (outcome.succeeded)
        {
            const int gainedLevels = outcome.wasCritical ? 2 : 1;
            progress.EnhanceSword(gainedLevels);
            progress.RecordSuccess();
            progress.GrantGold(65 + progress.GetSword().GetLevel() * 15);
        }
        else if (outcome.previousLevel < 4)
        {
            outcome.failureConsequence = FailureConsequence::LevelMaintained;
        }
        else if (progress.ConsumeFragment())
        {
            // 높은 단계에서는 기억 조각이 단계 하락보다 먼저 소비된다.
            outcome.failureConsequence = FailureConsequence::FragmentConsumed;
        }
        else
        {
            progress.DowngradeSword();
            outcome.failureConsequence = FailureConsequence::LevelLost;
        }

        outcome.newLevel = progress.GetSword().GetLevel();
        return outcome;
    }
}
