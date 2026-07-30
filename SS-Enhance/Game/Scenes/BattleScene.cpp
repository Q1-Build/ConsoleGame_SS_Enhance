#include "Game/Scenes/BattleScene.h"

#include "Game/Domain/BattleRules.h"
#include "Game/Domain/BattleSession.h"
#include "Game/Domain/PlayerProgress.h"
#include "Game/Domain/ProgressionRules.h"
#include "Game/Effects/ParticleSystem.h"
#include "Game/Scenes/GameHudRenderer.h"
#include "Game/Scenes/LocalizedText.h"
#include "Game/Scenes/SceneContext.h"
#include "Platform/IAudio.h"
#include "Platform/IInput.h"
#include "Rendering/IScreen.h"

#include <algorithm>
#include <cassert>
#include <optional>
#include <sstream>
#include <string_view>

namespace ss
{
    namespace
    {
        constexpr float kIntroductionSeconds = 1.4f;
        constexpr float kResultInputDelaySeconds = 0.65f;
        constexpr float kRewardInputDelaySeconds = 0.25f;

        // 보스의 전투 성격을 논리 음악 큐에 연결하고 실제 파일 선택은 플랫폼 계층에 남긴다.
        MusicTrack GetBattleMusic(BossType bossType) noexcept
        {
            switch (bossType)
            {
            case BossType::EmberWarden:
                return MusicTrack::BattleEmber;
            case BossType::StormSentinel:
                return MusicTrack::BattleStorm;
            case BossType::MemoryDevourer:
                return MusicTrack::BattleMemory;
            }
            assert(false && "지원하지 않는 보스 종류다.");
            return MusicTrack::BattleEmber;
        }
    }

    BattleScene::BattleScene(SceneContext& context)
        : context_(context)
    {
    }

    BattleScene::~BattleScene() = default;

    void BattleScene::OnEnter()
    {
        const std::optional<BossType> bossType = ProgressionRules::GetAvailableBoss(
            context_.progress.GetSword().GetLevel(),
            context_.progress.GetBossVictoryCount());
        assert(bossType.has_value());

        // 장면 진입 시 진행 상태로 보스를 확정해 전투 도중 외부 상태 변화에 영향받지 않게 한다.
        boss_ = BattleRules::GetBossDefinition(*bossType);
        session_ = std::make_unique<BattleSession>(
            boss_,
            context_.progress.GetSword().GetLevel());
        settlement_ = BattleSettlement{};
        phase_ = BattlePhase::Introduction;
        rewardChoice_ = RewardChoice::Gold;
        wasRetreat_ = false;
        phaseTimeSeconds_ = 0.0f;
        defeatTimeSeconds_ = 0.0f;
        messageTimeSeconds_ = 0.0f;
        shakeTimeSeconds_ = 0.0f;
        lastDamage_ = 0;
        battleMessage_.clear();
        context_.particles.Clear();
        context_.audio.PlayMusic(GetBattleMusic(boss_.type));
    }

    SceneTransition BattleScene::Update(float deltaSeconds)
    {
        assert(session_ != nullptr);
        assert(deltaSeconds >= 0.0f);

        messageTimeSeconds_ = std::max(0.0f, messageTimeSeconds_ - deltaSeconds);
        shakeTimeSeconds_ = std::max(0.0f, shakeTimeSeconds_ - deltaSeconds);
        phaseTimeSeconds_ += deltaSeconds;
        if (session_->IsComplete())
        {
            defeatTimeSeconds_ += deltaSeconds;
        }

        switch (phase_)
        {
        case BattlePhase::Introduction:
            return UpdateIntroduction(deltaSeconds);
        case BattlePhase::Combat:
            return UpdateCombat(deltaSeconds);
        case BattlePhase::RewardSelection:
            return UpdateRewardSelection(deltaSeconds);
        case BattlePhase::Completed:
            return UpdateCompleted(deltaSeconds);
        }
        return SceneTransition::None();
    }

    SceneTransition BattleScene::UpdateIntroduction(float deltaSeconds)
    {
        static_cast<void>(deltaSeconds);
        if (phaseTimeSeconds_ >= kIntroductionSeconds)
        {
            phase_ = BattlePhase::Combat;
            phaseTimeSeconds_ = 0.0f;
        }
        return SceneTransition::None();
    }

    SceneTransition BattleScene::UpdateCombat(float deltaSeconds)
    {
        if (context_.input.WasPressed(InputKey::Escape))
        {
            // 체력이 낮을 때 장면을 빠져나가 패배 페널티를 우회하지 못하도록 후퇴도 패배로 확정한다.
            BeginRetreat();
            return SceneTransition::None();
        }

        // 입력 순서는 방어 → 보스 공격 → 종료 확인 → 플레이어 공격으로 고정한다.
        if (context_.input.WasPressed(InputKey::A) ||
            context_.input.WasPressed(InputKey::Left))
        {
            const std::optional<GuardQuality> guard = session_->TryGuard();
            if (guard.has_value())
            {
                messageTimeSeconds_ = 0.55f;
                if (*guard == GuardQuality::Perfect)
                {
                    battleMessage_ = LocalizedText::Select(
                        context_.language,
                        L"완벽 방어 타이밍!",
                        L"PERFECT GUARD TIMING!");
                    messageColor_ = Color::BrightCyan;
                }
                else if (*guard == GuardQuality::Guarded)
                {
                    battleMessage_ = LocalizedText::Select(
                        context_.language,
                        L"방어 준비",
                        L"GUARD READY");
                    messageColor_ = Color::BrightYellow;
                }
                else
                {
                    battleMessage_ = LocalizedText::Select(
                        context_.language,
                        L"아직 공격이 오지 않습니다.",
                        L"TOO EARLY TO GUARD.");
                    messageColor_ = Color::BrightBlack;
                }
            }
        }

        const std::optional<BossAttackResult> bossAttackResult =
            session_->Update(deltaSeconds);
        if (bossAttackResult.has_value())
        {
            ShowBossAttackResult(*bossAttackResult);
        }

        // 보스 공격으로 체력이 0이 된 프레임에는 플레이어 공격을 더 이상 해석하지 않는다.
        if (session_->IsComplete())
        {
            BeginBattleResult();
            return SceneTransition::None();
        }

        if (context_.input.WasPressed(InputKey::Space) ||
            context_.input.WasPressed(InputKey::Enter))
        {
            const std::optional<int> damage = session_->TryAttack();
            if (damage.has_value())
            {
                context_.audio.PlaySound(SoundEffect::PlayerAttack);
                lastDamage_ = *damage;
                if (!bossAttackResult.has_value())
                {
                    battleMessage_ = std::wstring(LocalizedText::Select(
                        context_.language,
                        L"공격 적중! 피해 ",
                        L"STRIKE! DAMAGE "));
                    battleMessage_ += std::to_wstring(*damage);
                    messageColor_ = Color::BrightGreen;
                    messageTimeSeconds_ = 0.5f;
                    shakeTimeSeconds_ = 0.08f;
                }
                context_.particles.SpawnImpact(
                    static_cast<float>(*damage) / 50.0f);
            }
        }

        if (session_->IsComplete())
        {
            BeginBattleResult();
        }
        return SceneTransition::None();
    }

    SceneTransition BattleScene::UpdateRewardSelection(float deltaSeconds)
    {
        static_cast<void>(deltaSeconds);
        if (context_.input.WasPressed(InputKey::Left) ||
            context_.input.WasPressed(InputKey::A))
        {
            rewardChoice_ = RewardChoice::Gold;
            context_.audio.PlaySound(SoundEffect::MenuMove);
        }
        if (context_.input.WasPressed(InputKey::Right) ||
            context_.input.WasPressed(InputKey::D))
        {
            rewardChoice_ = RewardChoice::Memory;
            context_.audio.PlaySound(SoundEffect::MenuMove);
        }

        const bool isConfirmed =
            context_.input.WasPressed(InputKey::Enter) ||
            context_.input.WasPressed(InputKey::Space);
        if (phaseTimeSeconds_ >= kRewardInputDelaySeconds && isConfirmed)
        {
            ApplySelectedReward();
            context_.audio.PlaySound(SoundEffect::RewardConfirm);
            phase_ = BattlePhase::Completed;
            phaseTimeSeconds_ = 0.0f;
        }
        return SceneTransition::None();
    }

    SceneTransition BattleScene::UpdateCompleted(float deltaSeconds)
    {
        static_cast<void>(deltaSeconds);
        const bool isContinueRequested =
            context_.input.WasPressed(InputKey::Enter) ||
            context_.input.WasPressed(InputKey::Space);
        if (phaseTimeSeconds_ < kResultInputDelaySeconds || !isContinueRequested)
        {
            return SceneTransition::None();
        }

        if (session_->IsPlayerVictorious() &&
            ProgressionRules::IsEndingAchieved(
                context_.progress.GetBossVictoryCount()))
        {
            return SceneTransition::To(SceneType::Ending);
        }
        return SceneTransition::To(SceneType::Forge);
    }

    void BattleScene::ShowBossAttackResult(const BossAttackResult& result)
    {
        std::wstringstream resultMessage;
        if (result.wasFeint)
        {
            resultMessage << LocalizedText::Select(
                context_.language,
                L"보랏빛 잔상이 흩어졌습니다.",
                L"THE VIOLET AFTERIMAGE DISSOLVES.");
            messageColor_ = Color::BrightMagenta;
        }
        else if (result.guardQuality == GuardQuality::Perfect)
        {
            context_.audio.PlaySound(SoundEffect::PerfectGuard);
            resultMessage << LocalizedText::Select(
                                 context_.language,
                                 L"완벽 방어! 반격 ",
                                 L"PERFECT GUARD! COUNTER ")
                          << result.counterDamage;
            messageColor_ = Color::BrightCyan;
            shakeTimeSeconds_ = 0.12f;
        }
        else if (result.guardQuality == GuardQuality::Guarded)
        {
            context_.audio.PlaySound(SoundEffect::Guard);
            resultMessage << LocalizedText::Select(
                                 context_.language,
                                 L"방어 성공! 피해 ",
                                 L"GUARDED! DAMAGE ")
                          << result.damageTaken;
            messageColor_ = Color::BrightYellow;
            shakeTimeSeconds_ = 0.08f;
        }
        else
        {
            context_.audio.PlaySound(SoundEffect::PlayerHit);
            resultMessage << LocalizedText::Select(
                                 context_.language,
                                 L"피격! 피해 ",
                                 L"HIT! DAMAGE ")
                          << result.damageTaken;
            messageColor_ = Color::BrightRed;
            shakeTimeSeconds_ = 0.24f;
        }

        if (result.hasQuickFollowUp)
        {
            resultMessage << LocalizedText::Select(
                context_.language,
                L" | 다음 공격이 이어집니다!",
                L" | QUICK FOLLOW-UP!");
        }
        battleMessage_ = resultMessage.str();
        messageTimeSeconds_ = 0.85f;
    }

    void BattleScene::BeginBattleResult()
    {
        assert(session_->IsComplete());
        phaseTimeSeconds_ = 0.0f;
        defeatTimeSeconds_ = 0.0f;
        context_.particles.Clear();
        context_.particles.SpawnResultBurst(
            session_->IsPlayerVictorious(),
            bossRenderer_.GetPrimaryColor(boss_.type),
            75);

        if (session_->IsPlayerVictorious())
        {
            context_.audio.PlaySound(SoundEffect::BattleVictory);
            phase_ = BattlePhase::RewardSelection;
            return;
        }

        // 패배는 선택 단계가 없으므로 즉시 한 번만 강화 단계 하락을 확정한다.
        const bool wasApplied = settlement_.ApplyDefeat(context_.progress);
        assert(wasApplied);
        static_cast<void>(wasApplied);
        context_.audio.PlaySound(SoundEffect::BattleDefeat);
        phase_ = BattlePhase::Completed;
    }

    void BattleScene::BeginRetreat()
    {
        assert(phase_ == BattlePhase::Combat);

        const bool wasApplied = settlement_.ApplyDefeat(context_.progress);
        assert(wasApplied);
        static_cast<void>(wasApplied);
        context_.audio.PlaySound(SoundEffect::BattleDefeat);
        wasRetreat_ = true;
        phase_ = BattlePhase::Completed;
        phaseTimeSeconds_ = 0.0f;
        messageTimeSeconds_ = 0.0f;
        shakeTimeSeconds_ = 0.0f;
        context_.particles.Clear();
    }

    void BattleScene::ApplySelectedReward()
    {
        const bool wasApplied = settlement_.ApplyVictory(
            context_.progress,
            GetSelectedReward());
        assert(wasApplied);
        static_cast<void>(wasApplied);
    }

    BattleReward BattleScene::GetSelectedReward() const noexcept
    {
        return rewardChoice_ == RewardChoice::Gold
            ? boss_.goldReward
            : boss_.memoryReward;
    }

    void BattleScene::Render(IScreen& screen) const
    {
        assert(session_ != nullptr);

        context_.hudRenderer.DrawBackdrop(screen, context_.worldTimeSeconds);
        const int shakeX = shakeTimeSeconds_ > 0.0f &&
            static_cast<int>(context_.worldTimeSeconds * 45.0f) % 2 == 0
                ? 1
                : (shakeTimeSeconds_ > 0.0f ? -1 : 0);
        const Color bossColor = bossRenderer_.GetPrimaryColor(boss_.type);
        screen.Box(5 + shakeX, 4, 98 + shakeX, 32, Color::BrightBlack);
        screen.CenterText(
            6,
            LocalizedText::GetBossName(context_.language, boss_.type),
            bossColor);
        screen.CenterText(
            7,
            LocalizedText::GetBossPatternDescription(
                context_.language,
                boss_.type),
            Color::BrightBlack);

        context_.hudRenderer.DrawProgressBar(
            screen,
            12,
            9,
            28,
            static_cast<float>(session_->GetPlayerHealth()) /
                static_cast<float>(session_->GetPlayerMaxHealth()),
            Color::BrightGreen,
            LocalizedText::Select(context_.language, L"나   ", L"YOU  "));
        context_.hudRenderer.DrawProgressBar(
            screen,
            55,
            9,
            28,
            static_cast<float>(session_->GetBossHealth()) /
                static_cast<float>(session_->GetBossMaxHealth()),
            bossColor,
            LocalizedText::Select(context_.language, L"보스 ", L"BOSS "));

        const float introductionProgress = phase_ == BattlePhase::Introduction
            ? phaseTimeSeconds_ / kIntroductionSeconds
            : 1.0f;
        const float defeatProgress = session_->IsPlayerVictorious()
            ? defeatTimeSeconds_ / 0.8f
            : 0.0f;
        bossRenderer_.Draw(
            screen,
            boss_.type,
            introductionProgress,
            defeatProgress,
            shakeX,
            context_.worldTimeSeconds);

        if (phase_ == BattlePhase::Introduction)
        {
            screen.CenterText(
                24,
                LocalizedText::GetBossIntroduction(
                    context_.language,
                    boss_.type),
                bossColor);
            screen.CenterText(
                29,
                LocalizedText::Select(
                    context_.language,
                    L"전투 준비",
                    L"PREPARE FOR BATTLE"),
                Color::BrightYellow);
            return;
        }

        if (phase_ == BattlePhase::Combat)
        {
            DrawAttackWarning(screen);

            constexpr int kTimingWidth = 50;
            screen.Text(25, 21, L"[", Color::BrightBlack);
            const int markerPosition = static_cast<int>(
                session_->GetMarker() * static_cast<float>(kTimingWidth - 1));
            for (int index = 0; index < kTimingWidth; ++index)
            {
                const bool isCenter = index >= 22 && index <= 27;
                screen.Put(
                    26 + index,
                    21,
                    isCenter ? L'◆' : L'─',
                    isCenter ? Color::BrightGreen : Color::BrightBlack);
            }
            screen.Put(26 + markerPosition, 21, L'█', Color::BrightYellow);
            screen.Put(26 + kTimingWidth, 21, L']', Color::BrightBlack);
        }

        if (messageTimeSeconds_ > 0.0f && !battleMessage_.empty())
        {
            screen.CenterText(24, battleMessage_, messageColor_);
        }
        else if (lastDamage_ > 0 && phase_ == BattlePhase::Combat)
        {
            std::wstringstream damage;
            damage << LocalizedText::Select(
                          context_.language, L"마지막 피해 ", L"LAST DAMAGE ")
                   << lastDamage_;
            screen.CenterText(24, damage.str(), Color::BrightCyan);
        }

        if (phase_ == BattlePhase::Combat)
        {
            screen.CenterText(
                29,
                LocalizedText::Select(
                    context_.language,
                    L"[ SPACE ] 공격   [ A / ← ] 방어   [ ESC ] 후퇴(-1강)",
                    L"[ SPACE ] ATTACK   [ A / ← ] GUARD   [ ESC ] RETREAT(-1)"),
                Color::BrightYellow);
            return;
        }

        if (phase_ == BattlePhase::RewardSelection)
        {
            DrawRewardSelection(screen);
            return;
        }

        const bool didWin = session_->IsPlayerVictorious();
        std::wstring_view resultHeadline;
        if (didWin)
        {
            resultHeadline = LocalizedText::Select(
                context_.language,
                L"보상 획득 완료",
                L"REWARD CLAIMED");
        }
        else if (wasRetreat_)
        {
            resultHeadline = LocalizedText::Select(
                context_.language,
                L"전투에서 후퇴했습니다.",
                L"RETREATED FROM BATTLE.");
        }
        else
        {
            resultHeadline = LocalizedText::Select(
                context_.language,
                L"전투 패배",
                L"DEFEATED");
        }
        screen.CenterText(
            24,
            resultHeadline,
            didWin ? Color::BrightGreen : Color::BrightRed);
        if (didWin)
        {
            const BattleReward reward = GetSelectedReward();
            std::wstringstream rewardText;
            rewardText << L"+" << reward.gold << L" G  |  +"
                       << reward.fragments
                       << LocalizedText::Select(
                              context_.language,
                              L" 기억 조각",
                              L" MEMORY SHARDS");
            screen.CenterText(27, rewardText.str(), Color::BrightCyan);
        }
        else
        {
            std::wstringstream penalty;
            penalty << LocalizedText::Select(
                           context_.language,
                           L"강화 단계 감소: +",
                           L"SWORD DOWNGRADED TO +")
                    << context_.progress.GetSword().GetLevel();
            screen.CenterText(27, penalty.str(), Color::BrightRed);
        }
        screen.CenterText(
            30,
            LocalizedText::Select(
                context_.language,
                L"[ ENTER ] 계속",
                L"[ ENTER ] CONTINUE"),
            Color::BrightYellow);
    }

    void BattleScene::DrawAttackWarning(IScreen& screen) const
    {
        if (!session_->IsAttackWarning())
        {
            return;
        }

        const AttackTelegraph telegraph = session_->GetCurrentTelegraph();
        const bool isFeint = telegraph == AttackTelegraph::Feint;
        const std::wstring_view instruction = isFeint
            ? LocalizedText::Select(
                context_.language,
                L"보랏빛 잔상입니다. 방어하지 마세요.",
                L"VIOLET AFTERIMAGE. DO NOT GUARD.")
            : LocalizedText::Select(
                context_.language,
                L"[!] 표시가 초록 구간에 들어오면 A / ←",
                L"[!] PRESS A / ← IN THE GREEN ZONE");
        screen.CenterText(
            11,
            instruction,
            isFeint ? Color::BrightMagenta : Color::BrightYellow);

        constexpr int kGuardWidth = 42;
        constexpr int kGuardLeft = 30;
        const int markerPosition = static_cast<int>(
            session_->GetWarningProgress() *
            static_cast<float>(kGuardWidth - 1));
        const int perfectStart = static_cast<int>(
            session_->GetPerfectGuardStartProgress() *
            static_cast<float>(kGuardWidth - 1));

        screen.Put(kGuardLeft, 12, L'[', Color::BrightBlack);
        for (int index = 0; index < kGuardWidth; ++index)
        {
            const bool isResponseZone = index >= perfectStart;
            screen.Put(
                kGuardLeft + 1 + index,
                12,
                isResponseZone ? L'◆' : L'─',
                isFeint
                    ? Color::BrightMagenta
                    : (isResponseZone ? Color::BrightGreen : Color::BrightBlack));
        }
        screen.Put(
            kGuardLeft + 1 + markerPosition,
            12,
            L'█',
            isFeint
                ? Color::BrightMagenta
                : (session_->IsPerfectGuardWindow()
                    ? Color::BrightCyan
                    : Color::BrightYellow));
        screen.Put(kGuardLeft + 1 + kGuardWidth, 12, L']', Color::BrightBlack);

        std::wstringstream step;
        step << LocalizedText::Select(context_.language, L"공격 순서 ", L"SEQUENCE ")
             << session_->GetCurrentAttackStep() + 1
             << L"/"
             << session_->GetAttackStepCount();
        screen.CenterText(19, step.str(), Color::BrightBlack);
    }

    void BattleScene::DrawRewardSelection(IScreen& screen) const
    {
        screen.CenterText(
            21,
            LocalizedText::GetBossDefeatText(
                context_.language,
                boss_.type),
            bossRenderer_.GetPrimaryColor(boss_.type));
        screen.CenterText(
            23,
            LocalizedText::Select(
                context_.language,
                L"승리의 대가를 선택하세요.",
                L"CHOOSE YOUR VICTORY REWARD."),
            Color::BrightGreen);

        std::wstringstream goldReward;
        goldReward << (rewardChoice_ == RewardChoice::Gold ? L"▶ " : L"  ")
                   << LocalizedText::Select(
                          context_.language,
                          L"길드 보수  ",
                          L"GUILD PAY  ")
                   << boss_.goldReward.gold << L" G";
        screen.Text(
            18,
            26,
            goldReward.str(),
            rewardChoice_ == RewardChoice::Gold
                ? Color::BrightYellow
                : Color::BrightBlack);

        std::wstringstream memoryReward;
        memoryReward << (rewardChoice_ == RewardChoice::Memory ? L"▶ " : L"  ")
                     << LocalizedText::Select(
                            context_.language,
                            L"기억 회수  ",
                            L"MEMORY CACHE  ")
                     << boss_.memoryReward.gold << L" G + "
                     << boss_.memoryReward.fragments
                     << LocalizedText::Select(
                            context_.language,
                            L" 조각",
                            L" SHARDS");
        screen.Text(
            56,
            26,
            memoryReward.str(),
            rewardChoice_ == RewardChoice::Memory
                ? Color::BrightCyan
                : Color::BrightBlack);
        screen.CenterText(
            30,
            LocalizedText::Select(
                context_.language,
                L"[ ← / → ] 선택   [ ENTER ] 확정",
                L"[ ← / → ] SELECT   [ ENTER ] CONFIRM"),
            Color::BrightYellow);
    }

    void BattleScene::OnExit()
    {
        // 전투 전용 충격 파티클은 다른 장면으로 넘기지 않고 세션과 함께 정리한다.
        context_.particles.Clear();
        session_.reset();
    }
}
