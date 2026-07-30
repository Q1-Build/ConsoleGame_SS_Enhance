#include "Game/Domain/BattleRules.h"
#include "Game/Domain/BattleSession.h"
#include "Game/Domain/BattleSettlement.h"
#include "Game/Domain/ForgeRules.h"
#include "Game/Domain/ForgeSession.h"
#include "Game/Domain/PlayerProgress.h"
#include "Game/Domain/ProgressionRules.h"
#include "Game/Scenes/ChatOverlay.h"
#include "Platform/IInput.h"
#include "Rendering/ScreenBuffer.h"

#include <array>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace
{
    /// 프레임별 키와 문자 입력을 직접 구성해 입력 경계 동작을 검증하는 테스트 대역이다.
    class StubInput final : public ss::IInput
    {
    public:
        void Update() override
        {
        }

        [[nodiscard]] bool IsDown(ss::InputKey key) const override
        {
            return isDown_[static_cast<std::size_t>(key)];
        }

        [[nodiscard]] bool WasPressed(ss::InputKey key) const override
        {
            return wasPressed_[static_cast<std::size_t>(key)];
        }

        [[nodiscard]] std::wstring_view GetTextInput() const noexcept override
        {
            return textInput_;
        }

        void SetFrame(ss::InputKey key, std::wstring text = {})
        {
            isDown_.fill(false);
            wasPressed_.fill(false);
            isDown_[static_cast<std::size_t>(key)] = true;
            wasPressed_[static_cast<std::size_t>(key)] = true;
            textInput_ = std::move(text);
        }

        void SetText(std::wstring text)
        {
            isDown_.fill(false);
            wasPressed_.fill(false);
            textInput_ = std::move(text);
        }

        void Clear()
        {
            isDown_.fill(false);
            wasPressed_.fill(false);
            textInput_.clear();
        }

    private:
        std::array<
            bool,
            static_cast<std::size_t>(ss::InputKey::Count)> isDown_{};
        std::array<
            bool,
            static_cast<std::size_t>(ss::InputKey::Count)> wasPressed_{};
        std::wstring textInput_;
    };

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

        // 완벽 방어 구간보다 일찍 입력해 일반 방어가 피해 감소 규칙만 적용하는지 확인한다.
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

        // 가짜 예고는 피해 없이 끝나되 다음 치명타가 빠른 후속 공격으로 연결되어야 한다.
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

    void TestAttackTimingPreservesFrameOvershoot()
    {
        ss::BattleSession session(
            MakeBoss(
                100,
                {{1.0f, 0.5f, 10, ss::AttackTelegraph::Honest}}),
            4);

        // 첫 공격을 0.1초 지나 처리해도 초과 시간이 다음 공격 주기에서 사라지지 않아야 한다.
        Expect(
            session.Update(1.1f).has_value(),
            "first delayed attack did not resolve");
        Expect(
            !session.Update(0.85f).has_value(),
            "next attack resolved before its carried-over delay");
        Expect(
            session.Update(0.06f).has_value(),
            "frame overshoot was discarded from the next attack");
    }

    void TestBossDefinitionsHaveDistinctSequences()
    {
        // 보스별 공격 수와 속임수 위치를 고정해 데이터 정의가 같은 패턴으로 퇴행하지 않게 한다.
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
        // 장면 갱신에서 정산이 반복 호출되어도 보상과 진행 기록은 한 번만 반영되어야 한다.
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

        // 패배 정산도 같은 일회성 계약을 지켜 강화 단계가 중복 감소하지 않는지 함께 검증한다.
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

    void TestForgeHeatRanges()
    {
        Expect(
            ss::ForgeSession::IsOptimalHeat(64.0f) &&
            ss::ForgeSession::IsOptimalHeat(68.0f) &&
            ss::ForgeSession::IsOptimalHeat(72.0f),
            "optimal forge heat range rejected a boundary or center");
        Expect(
            !ss::ForgeSession::IsOptimalHeat(63.9f) &&
            !ss::ForgeSession::IsOptimalHeat(72.1f),
            "optimal forge heat range accepted an outside value");
        Expect(
            ss::ForgeSession::IsResonantHeat(58.0f) &&
            ss::ForgeSession::IsResonantHeat(78.0f),
            "resonant forge heat range rejected a boundary");
    }

    void TestChatOverlayScrollsVisibleLines()
    {
        ss::ChatOverlay chat;
        ss::ScreenBuffer screen;
        StubInput input;

        Expect(
            !chat.Update(input, screen),
            "idle chat consumed game input");

        for (int index = 0; index < 7; ++index)
        {
            input.SetFrame(ss::InputKey::T, L"t");
            Expect(
                chat.Update(input, screen),
                "chat did not consume its open command");

            input.SetText(L"note " + std::to_wstring(index));
            Expect(
                chat.Update(input, screen),
                "chat did not consume text editing");

            input.SetFrame(ss::InputKey::Enter);
            Expect(
                chat.Update(input, screen),
                "chat did not consume submission");
            input.Clear();
        }

        screen.Clear();
        chat.Draw(screen, ss::Language::English);
        const std::wstring frame = screen.BuildAnsiFrame();
        Expect(
            frame.find(L"note 0") == std::wstring::npos,
            "chat retained a line above visible capacity");
        Expect(
            frame.find(L"note 1") != std::wstring::npos &&
            frame.find(L"note 6") != std::wstring::npos,
            "chat did not retain the six newest lines");
    }

    void TestChatOverlayIgnoresDelayedOpenCharacter()
    {
        ss::ChatOverlay chat;
        ss::ScreenBuffer screen;
        StubInput input;

        input.SetFrame(ss::InputKey::T);
        Expect(
            chat.Update(input, screen),
            "chat did not consume its open command");

        // 한글 IME가 채팅 시작 키의 자모를 다음 프레임에 늦게 전달하는 실제 순서를 재현한다.
        input.SetText(L"ㅅ안녕하세요");
        Expect(
            chat.Update(input, screen),
            "chat did not consume delayed IME text");

        input.SetFrame(ss::InputKey::Enter);
        Expect(
            chat.Update(input, screen),
            "chat did not consume submission");

        screen.Clear();
        chat.Draw(screen, ss::Language::English);
        const std::wstring frame = screen.BuildAnsiFrame();
        Expect(
            frame.find(L"안녕하세요") != std::wstring::npos,
            "chat discarded message text after the delayed open character");
        Expect(
            frame.find(L"ㅅ안녕하세요") == std::wstring::npos,
            "chat retained the delayed Korean open character");
    }

    void TestDifficultyFailurePenaltyThresholds()
    {
        // 난이도별 페널티 시작 단계의 직전 값과 경계값을 함께 검사해 기준 변경을 탐지한다.
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

        // 보호용 기억 조각이 없을 때만 단계 하락으로 이어지는 페널티 우선순위를 확인한다.
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

    void TestProgressionBoundaries()
    {
        // 보스 승리 횟수에 따른 +4, +8, +12 상한을 경계 밖 입력까지 포함해 고정한다.
        Expect(
            ss::ProgressionRules::GetSwordLevelCap(-1) == 4,
            "negative victory count changed cap");
        Expect(
            ss::ProgressionRules::GetSwordLevelCap(0) == 4,
            "initial cap is not +4");
        Expect(
            ss::ProgressionRules::GetSwordLevelCap(1) == 8,
            "first victory did not unlock +8");
        Expect(
            ss::ProgressionRules::GetSwordLevelCap(2) == 12,
            "second victory did not unlock +12");
        Expect(
            ss::ProgressionRules::GetSwordLevelCap(3) == 12,
            "final cap exceeded +12");

        // 각 강화 구간의 보스 해금과 최종 보스 이후 엔딩 전환이 같은 진행 순서를 따르는지 검증한다.
        Expect(
            !ss::ProgressionRules::GetAvailableBoss(3, 0).has_value(),
            "ember boss unlocked below +4");
        Expect(
            ss::ProgressionRules::GetAvailableBoss(4, 0) ==
                ss::BossType::EmberWarden,
            "ember boss did not unlock at +4");
        Expect(
            ss::ProgressionRules::GetAvailableBoss(8, 1) ==
                ss::BossType::StormSentinel,
            "storm boss did not unlock at +8");
        Expect(
            ss::ProgressionRules::GetAvailableBoss(12, 2) ==
                ss::BossType::MemoryDevourer,
            "memory boss did not unlock at +12");
        Expect(
            !ss::ProgressionRules::GetAvailableBoss(12, 3).has_value(),
            "boss remained available after the ending");
        Expect(
            !ss::ProgressionRules::IsEndingAchieved(2),
            "ending unlocked before all bosses were defeated");
        Expect(
            ss::ProgressionRules::IsEndingAchieved(3),
            "ending did not unlock after all bosses were defeated");
    }

    template <typename TestFunction>
    void RunTest(
        const char* testName,
        TestFunction testFunction,
        int& passedCount,
        int& failedCount)
    {
        // 외부 테스트 프레임워크 없이 예외를 실패 신호로 사용해 모든 테스트를 끝까지 실행한다.
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
        "AttackTimingPreservesFrameOvershoot",
        TestAttackTimingPreservesFrameOvershoot,
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
        "ChatOverlayScrollsVisibleLines",
        TestChatOverlayScrollsVisibleLines,
        passedCount,
        failedCount);
    RunTest(
        "ChatOverlayIgnoresDelayedOpenCharacter",
        TestChatOverlayIgnoresDelayedOpenCharacter,
        passedCount,
        failedCount);
    RunTest(
        "ForgeHeatRanges",
        TestForgeHeatRanges,
        passedCount,
        failedCount);
    RunTest(
        "DifficultyFailurePenaltyThresholds",
        TestDifficultyFailurePenaltyThresholds,
        passedCount,
        failedCount);
    RunTest(
        "ProgressionBoundaries",
        TestProgressionBoundaries,
        passedCount,
        failedCount);

    std::cout << passedCount << " passed, " << failedCount << " failed\n";
    return failedCount == 0 ? 0 : 1;
}
