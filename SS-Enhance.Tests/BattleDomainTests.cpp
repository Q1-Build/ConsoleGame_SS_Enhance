#include "Game/Domain/BattleRules.h"
#include "Game/Domain/BattleSession.h"
#include "Game/Domain/BattleSettlement.h"
#include "Game/Domain/ForgeRules.h"
#include "Game/Domain/PlayerProgress.h"

#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace
{
    void Expect(bool condition, const std::string& message)
    {
        if (!condition)
        {
            throw std::runtime_error(message);
        }
    }

    ss::BossDefinition MakeBoss(
        int maxHealth,
        std::vector<ss::BossAttackStep> attackSequence)
    {
        return {
            ss::BossType::EmberWarden,
            maxHealth,
            std::move(attackSequence),
            {100, 0},
            {0, 1}};
    }

    void TestPerfectGuardBlocksAndCounters()
    {
        ss::BattleSession session(
            MakeBoss(
                100,
                {{1.0f, 0.5f, 20, ss::AttackTelegraph::Honest}}),
            4);

        // 공격 0.17초 전에 방어해 완벽 방어의 피해 무효화와 반격을 함께 검증한다.
        static_cast<void>(session.Update(0.83f));
        const std::optional<ss::GuardQuality> guard = session.TryGuard();
        Expect(
            guard == ss::GuardQuality::Perfect,
            "perfect guard timing was not recognized");

        const std::optional<ss::BossAttackResult> result = session.Update(0.18f);
        Expect(result.has_value(), "boss attack did not resolve");
        Expect(result->damageTaken == 0, "perfect guard dealt player damage");
        Expect(result->counterDamage > 0, "perfect guard did not counter");
        Expect(session.GetPlayerHealth() == session.GetPlayerMaxHealth(), "player health changed");
        Expect(session.GetBossHealth() < session.GetBossMaxHealth(), "boss health did not change");
    }

    void TestStandardGuardReducesDamage()
    {
        ss::BattleSession session(
            MakeBoss(
                100,
                {{1.0f, 0.6f, 20, ss::AttackTelegraph::Honest}}),
            4);

        static_cast<void>(session.Update(0.5f));
        const std::optional<ss::GuardQuality> guard = session.TryGuard();
        Expect(guard == ss::GuardQuality::Guarded, "standard guard was not prepared");

        const std::optional<ss::BossAttackResult> result = session.Update(0.5f);
        Expect(result.has_value(), "guarded attack did not resolve");
        Expect(
            result->damageTaken == ss::BattleRules::CalculateGuardedDamage(20),
            "guarded damage did not use BattleRules");
    }

    void TestFeintAndFatalFollowUp()
    {
        ss::BattleSession session(
            MakeBoss(
                100,
                {
                    {0.5f, 0.3f, 0, ss::AttackTelegraph::Feint},
                    {0.5f, 0.3f, 100, ss::AttackTelegraph::Honest}
                }),
            4);

        const std::optional<ss::BossAttackResult> feint = session.Update(0.5f);
        Expect(feint.has_value() && feint->wasFeint, "feint step was not reported");
        Expect(session.GetPlayerHealth() == 100, "feint damaged the player");
        Expect(feint->hasQuickFollowUp, "quick follow-up was not reported");

        const std::optional<ss::BossAttackResult> fatalAttack = session.Update(0.5f);
        Expect(fatalAttack.has_value(), "fatal follow-up did not resolve");
        Expect(session.IsComplete(), "zero player health did not complete battle");
        Expect(!session.IsPlayerVictorious(), "player death was treated as victory");
        Expect(!session.TryAttack().has_value(), "attack was accepted after player death");
    }

    void TestBossDefinitionsHaveDistinctSequences()
    {
        const ss::BossDefinition ember =
            ss::BattleRules::GetBossDefinition(ss::BossType::EmberWarden);
        const ss::BossDefinition storm =
            ss::BattleRules::GetBossDefinition(ss::BossType::StormSentinel);
        const ss::BossDefinition memory =
            ss::BattleRules::GetBossDefinition(ss::BossType::MemoryDevourer);

        Expect(ember.attackSequence.size() == 1, "ember sequence changed unexpectedly");
        Expect(storm.attackSequence.size() == 3, "storm is not a triple sequence");
        Expect(memory.attackSequence.size() == 3, "memory sequence is incomplete");
        Expect(
            memory.attackSequence[1].telegraph == ss::AttackTelegraph::Feint,
            "memory boss has no readable feint");
    }

    void TestSettlementAppliesOnce()
    {
        ss::PlayerProgress victoryProgress;
        ss::BattleSettlement victorySettlement;
        const int initialGold = victoryProgress.GetGold();
        const int initialFragments = victoryProgress.GetFragments();

        Expect(
            victorySettlement.ApplyVictory(victoryProgress, {250, 2}),
            "first victory settlement failed");
        Expect(
            !victorySettlement.ApplyVictory(victoryProgress, {250, 2}),
            "victory settlement was applied twice");
        Expect(victorySettlement.IsApplied(), "victory settlement did not retain completion");
        Expect(victoryProgress.GetGold() == initialGold + 250, "victory gold duplicated");
        Expect(
            victoryProgress.GetFragments() == initialFragments + 2,
            "victory fragments duplicated");
        Expect(victoryProgress.GetBossVictoryCount() == 1, "boss victory duplicated");

        ss::PlayerProgress defeatProgress;
        defeatProgress.EnhanceSword(4);
        ss::BattleSettlement defeatSettlement;
        Expect(defeatSettlement.ApplyDefeat(defeatProgress), "first defeat settlement failed");
        Expect(!defeatSettlement.ApplyDefeat(defeatProgress), "defeat settlement was applied twice");
        Expect(defeatSettlement.IsApplied(), "defeat settlement did not retain completion");
        Expect(defeatProgress.GetSword().GetLevel() == 3, "defeat penalty was not exactly one level");
    }

    ss::ForgeOutcome ResolveFailedForge(
        ss::PlayerProgress& progress,
        int swordLevel,
        ss::Difficulty difficulty)
    {
        // 최종 강화 구간까지 해금해 단계 상한이 실패 규칙 테스트를 가로막지 않게 한다.
        progress.RecordBossVictory();
        progress.RecordBossVictory();
        progress.EnhanceSword(swordLevel);

        ss::ForgeRules rules;
        return rules.Resolve(
            progress,
            {0.0f, 0.0f, 0.0f},
            difficulty,
            1.0f);
    }

    void TestDifficultyFailurePenaltyThresholds()
    {
        ss::PlayerProgress hardSafeProgress;
        const ss::ForgeOutcome hardSafe = ResolveFailedForge(
            hardSafeProgress,
            3,
            ss::Difficulty::Hard);
        Expect(
            hardSafe.failureConsequence == ss::FailureConsequence::LevelMaintained,
            "hard penalty started below +4");

        ss::PlayerProgress hardPenaltyProgress;
        const ss::ForgeOutcome hardPenalty = ResolveFailedForge(
            hardPenaltyProgress,
            4,
            ss::Difficulty::Hard);
        Expect(
            hardPenalty.failureConsequence == ss::FailureConsequence::FragmentConsumed,
            "hard penalty did not start at +4");

        ss::PlayerProgress normalSafeProgress;
        const ss::ForgeOutcome normalSafe = ResolveFailedForge(
            normalSafeProgress,
            8,
            ss::Difficulty::Normal);
        Expect(
            normalSafe.failureConsequence == ss::FailureConsequence::LevelMaintained,
            "normal penalty started below +9");

        ss::PlayerProgress normalPenaltyProgress;
        const ss::ForgeOutcome normalPenalty = ResolveFailedForge(
            normalPenaltyProgress,
            9,
            ss::Difficulty::Normal);
        Expect(
            normalPenalty.failureConsequence == ss::FailureConsequence::FragmentConsumed,
            "normal penalty did not start at +9");

        ss::PlayerProgress normalLossProgress;
        static_cast<void>(normalLossProgress.ConsumeFragment());
        static_cast<void>(normalLossProgress.ConsumeFragment());
        const ss::ForgeOutcome normalLoss = ResolveFailedForge(
            normalLossProgress,
            9,
            ss::Difficulty::Normal);
        Expect(
            normalLoss.failureConsequence == ss::FailureConsequence::LevelLost,
            "normal failure without fragments did not lose a level");
        Expect(
            normalLossProgress.GetSword().GetLevel() == 8,
            "normal level loss did not reduce +9 to +8");

        ss::PlayerProgress easySafeProgress;
        const ss::ForgeOutcome easySafe = ResolveFailedForge(
            easySafeProgress,
            9,
            ss::Difficulty::Easy);
        Expect(
            easySafe.failureConsequence == ss::FailureConsequence::LevelMaintained,
            "easy penalty started below +10");

        ss::PlayerProgress easyPenaltyProgress;
        const ss::ForgeOutcome easyPenalty = ResolveFailedForge(
            easyPenaltyProgress,
            10,
            ss::Difficulty::Easy);
        Expect(
            easyPenalty.failureConsequence == ss::FailureConsequence::FragmentConsumed,
            "easy penalty did not start at +10");
    }

    template <typename TestFunction>
    void RunTest(
        const char* testName,
        TestFunction testFunction,
        int& passedCount,
        int& failedCount)
    {
        try
        {
            testFunction();
            ++passedCount;
            std::cout << "[PASS] " << testName << '\n';
        }
        catch (const std::exception& exception)
        {
            ++failedCount;
            std::cout << "[FAIL] " << testName << ": " << exception.what() << '\n';
        }
    }
}

int main()
{
    int passedCount = 0;
    int failedCount = 0;

    RunTest(
        "PerfectGuardBlocksAndCounters",
        TestPerfectGuardBlocksAndCounters,
        passedCount,
        failedCount);
    RunTest(
        "StandardGuardReducesDamage",
        TestStandardGuardReducesDamage,
        passedCount,
        failedCount);
    RunTest(
        "FeintAndFatalFollowUp",
        TestFeintAndFatalFollowUp,
        passedCount,
        failedCount);
    RunTest(
        "BossDefinitionsHaveDistinctSequences",
        TestBossDefinitionsHaveDistinctSequences,
        passedCount,
        failedCount);
    RunTest(
        "SettlementAppliesOnce",
        TestSettlementAppliesOnce,
        passedCount,
        failedCount);
    RunTest(
        "DifficultyFailurePenaltyThresholds",
        TestDifficultyFailurePenaltyThresholds,
        passedCount,
        failedCount);

    std::cout << passedCount << " passed, " << failedCount << " failed\n";
    return failedCount == 0 ? 0 : 1;
}
