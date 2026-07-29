#include "Game/Scenes/BattleScene.h"

#include "Game/Domain/BattleRules.h"
#include "Game/Domain/BattleSession.h"
#include "Game/Domain/PlayerProgress.h"
#include "Game/Domain/ProgressionRules.h"
#include "Game/Effects/ParticleSystem.h"
#include "Game/Scenes/GameHudRenderer.h"
#include "Game/Scenes/LocalizedText.h"
#include "Game/Scenes/SceneContext.h"
#include "Platform/IInput.h"
#include "Rendering/IScreen.h"

#include <algorithm>
#include <cassert>
#include <optional>
#include <sstream>

namespace ss
{
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

        // 장면 진입 시 진행 상태로 보스를 한 번 확정해 전투 도중 강화 상태 변화에 영향받지 않게 한다.
        boss_ = BattleRules::GetBossDefinition(*bossType);
        session_ = std::make_unique<BattleSession>(
            boss_,
            context_.progress.GetSword().GetLevel());
        completedTimeSeconds_ = 0.0f;
        messageTimeSeconds_ = 0.0f;
        shakeTimeSeconds_ = 0.0f;
        lastDamage_ = 0;
        battleMessage_.clear();
        wasBattleOutcomeApplied_ = false;
        context_.particles.Clear();
    }

    SceneTransition BattleScene::Update(float deltaSeconds)
    {
        assert(session_ != nullptr);

        if (!session_->IsComplete())
        {
            messageTimeSeconds_ = std::max(0.0f, messageTimeSeconds_ - deltaSeconds);
            shakeTimeSeconds_ = std::max(0.0f, shakeTimeSeconds_ - deltaSeconds);

            if (context_.input.WasPressed(InputKey::Escape))
            {
                return SceneTransition::To(SceneType::Forge);
            }

            // 입력 순서는 방어 → 보스 공격 → 종료 확인 → 플레이어 공격으로 고정한다.
            // 방어 입력을 공격 갱신보다 먼저 처리해 판정 직전 프레임의 입력도 놓치지 않는다.
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
                std::wstringstream resultMessage;
                if (bossAttackResult->guardQuality == GuardQuality::Perfect)
                {
                    resultMessage << LocalizedText::Select(
                                         context_.language,
                                         L"완벽 방어! 반격 ",
                                         L"PERFECT GUARD! COUNTER ")
                                  << bossAttackResult->counterDamage;
                    messageColor_ = Color::BrightCyan;
                    shakeTimeSeconds_ = 0.12f;
                }
                else if (bossAttackResult->guardQuality == GuardQuality::Guarded)
                {
                    resultMessage << LocalizedText::Select(
                                         context_.language,
                                         L"방어 성공! 피해 ",
                                         L"GUARDED! DAMAGE ")
                                  << bossAttackResult->damageTaken;
                    messageColor_ = Color::BrightYellow;
                    shakeTimeSeconds_ = 0.08f;
                }
                else
                {
                    resultMessage << LocalizedText::Select(
                                         context_.language,
                                         L"피격! 피해 ",
                                         L"HIT! DAMAGE ")
                                  << bossAttackResult->damageTaken;
                    messageColor_ = Color::BrightRed;
                    shakeTimeSeconds_ = 0.24f;
                }

                if (bossAttackResult->hasMoreComboAttacks)
                {
                    resultMessage << LocalizedText::Select(
                        context_.language,
                        L" | 연속 공격!",
                        L" | COMBO CONTINUES!");
                }
                battleMessage_ = resultMessage.str();
                messageTimeSeconds_ = 0.85f;
            }

            // 보스 공격으로 체력이 0이 된 프레임에는 플레이어 공격을 더 이상 해석하지 않는다.
            if (session_->IsComplete())
            {
                ApplyBattleOutcome();
                return SceneTransition::None();
            }

            if (context_.input.WasPressed(InputKey::Space) ||
                context_.input.WasPressed(InputKey::Enter))
            {
                const std::optional<int> damage = session_->TryAttack();
                if (damage.has_value())
                {
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
                ApplyBattleOutcome();
            }
            return SceneTransition::None();
        }

        // 결과는 완료 프레임 수와 관계없이 한 번만 진행도에 반영한다.
        completedTimeSeconds_ += deltaSeconds;
        ApplyBattleOutcome();

        const bool isContinueRequested =
            context_.input.WasPressed(InputKey::Enter) ||
            context_.input.WasPressed(InputKey::Space);
        if (completedTimeSeconds_ >= 0.65f && isContinueRequested)
        {
            if (session_->IsPlayerVictorious() &&
                ProgressionRules::IsEndingAchieved(
                    context_.progress.GetBossVictoryCount()))
            {
                return SceneTransition::To(SceneType::Ending);
            }
            return SceneTransition::To(SceneType::Forge);
        }
        return SceneTransition::None();
    }

    void BattleScene::Render(IScreen& screen) const
    {
        assert(session_ != nullptr);

        context_.hudRenderer.DrawBackdrop(screen, context_.worldTimeSeconds);
        const int shakeX = shakeTimeSeconds_ > 0.0f &&
            static_cast<int>(context_.worldTimeSeconds * 45.0f) % 2 == 0
                ? 1
                : (shakeTimeSeconds_ > 0.0f ? -1 : 0);
        screen.Box(5 + shakeX, 4, 98 + shakeX, 30, Color::BrightBlack);
        screen.CenterText(
            6,
            LocalizedText::GetBossName(context_.language, boss_.type),
            Color::BrightRed);
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
            Color::BrightRed,
            LocalizedText::Select(context_.language, L"보스 ", L"BOSS "));

        if (session_->IsAttackWarning() && !session_->IsComplete())
        {
            screen.CenterText(
                11,
                LocalizedText::Select(
                    context_.language,
                    L"[!] 표시가 초록 구간에 들어오면 A / ←",
                    L"[!] PRESS A / ← IN THE GREEN ZONE"),
                session_->IsPerfectGuardWindow()
                    ? Color::BrightCyan
                    : Color::BrightYellow);

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
                const bool isPerfectZone = index >= perfectStart;
                screen.Put(
                    kGuardLeft + 1 + index,
                    12,
                    isPerfectZone ? L'◆' : L'─',
                    isPerfectZone ? Color::BrightGreen : Color::BrightBlack);
            }
            screen.Put(
                kGuardLeft + 1 + markerPosition,
                12,
                L'█',
                session_->IsPerfectGuardWindow()
                    ? Color::BrightCyan
                    : Color::BrightYellow);
            screen.Put(
                kGuardLeft + 1 + kGuardWidth,
                12,
                L']',
                Color::BrightBlack);
        }

        // 피격과 반격 시 보스 형상과 전투 프레임을 함께 흔들어 판정의 충격을 전달한다.
        screen.Text(43 + shakeX, 13, L"       ╱╲       ", Color::BrightRed);
        screen.Text(43 + shakeX, 14, L"   ╭──╯  ╰──╮   ", Color::BrightRed);
        screen.Text(43 + shakeX, 15, L"   │  ◆  ◆  │   ", Color::BrightYellow);
        screen.Text(43 + shakeX, 16, L"   ╰──╮  ╭──╯   ", Color::BrightRed);
        screen.Text(43 + shakeX, 17, L"      ╰──╯      ", Color::BrightBlack);

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

        // 최근 판정 메시지는 공격 피해보다 우선해 방어와 연속 공격 결과를 놓치지 않게 한다.
        if (messageTimeSeconds_ > 0.0f && !battleMessage_.empty())
        {
            screen.CenterText(24, battleMessage_, messageColor_);
        }
        else if (lastDamage_ > 0)
        {
            std::wstringstream damage;
            damage << LocalizedText::Select(
                          context_.language, L"마지막 피해 ", L"LAST DAMAGE ")
                   << lastDamage_;
            screen.CenterText(24, damage.str(), Color::BrightCyan);
        }

        if (!session_->IsComplete())
        {
            screen.CenterText(
                27,
                LocalizedText::Select(
                    context_.language,
                    L"[ SPACE ] 공격   [ A / ← ] 방어   [ ESC ] 후퇴",
                    L"[ SPACE ] ATTACK   [ A / ← ] GUARD   [ ESC ] RETREAT"),
                Color::BrightYellow);
            return;
        }

        const bool didWin = session_->IsPlayerVictorious();
        screen.CenterText(
            25,
            didWin
                ? LocalizedText::Select(context_.language, L"보스 격파", L"BOSS DEFEATED")
                : LocalizedText::Select(context_.language, L"전투 패배", L"DEFEATED"),
            didWin ? Color::BrightGreen : Color::BrightRed);
        if (didWin)
        {
            std::wstringstream reward;
            reward << L"+" << boss_.rewardGold << L" G  |  +"
                   << boss_.rewardFragments
                   << LocalizedText::Select(
                          context_.language,
                          L" 기억 조각",
                          L" MEMORY SHARDS");
            screen.CenterText(27, reward.str(), Color::BrightCyan);
        }
        else
        {
            std::wstringstream penalty;
            penalty << LocalizedText::Select(
                           context_.language,
                           L"강화 단계 감소: +",
                           L"ENHANCEMENT LOST: +")
                    << context_.progress.GetSword().GetLevel();
            screen.CenterText(27, penalty.str(), Color::BrightRed);
        }
        screen.CenterText(
            29,
            LocalizedText::Select(
                context_.language,
                L"[ ENTER ] 계속",
                L"[ ENTER ] CONTINUE"),
            Color::BrightYellow);
    }

    void BattleScene::OnExit()
    {
        // 전투 전용 충격 파티클은 다른 장면으로 넘기지 않고 세션과 함께 정리한다.
        context_.particles.Clear();
        session_.reset();
    }

    void BattleScene::ApplyBattleOutcome()
    {
        assert(session_ != nullptr);
        assert(session_->IsComplete());
        if (wasBattleOutcomeApplied_)
        {
            return;
        }

        // 승패 결과를 한 경계에서 확정해 보상이나 패배 페널티가 중복 적용되지 않게 한다.
        if (session_->IsPlayerVictorious())
        {
            context_.progress.GrantGold(boss_.rewardGold);
            context_.progress.GrantFragments(boss_.rewardFragments);
            context_.progress.RecordBossVictory();
        }
        else
        {
            context_.progress.DowngradeSword();
        }
        wasBattleOutcomeApplied_ = true;
    }
}
