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
        lastDamage_ = 0;
        wasRewardGranted_ = false;
        context_.particles.Clear();
    }

    SceneTransition BattleScene::Update(float deltaSeconds)
    {
        assert(session_ != nullptr);

        if (!session_->IsComplete())
        {
            if (context_.input.WasPressed(InputKey::Escape))
            {
                return SceneTransition::To(SceneType::Forge);
            }

            session_->Update(deltaSeconds);
            if (context_.input.WasPressed(InputKey::Space) ||
                context_.input.WasPressed(InputKey::Enter))
            {
                const std::optional<int> damage = session_->TryAttack();
                if (damage.has_value())
                {
                    lastDamage_ = *damage;
                    context_.particles.SpawnImpact(
                        static_cast<float>(*damage) / 50.0f);
                }
            }
            return SceneTransition::None();
        }

        // 보상은 완료 프레임 수와 관계없이 한 번만 진행도에 반영한다.
        completedTimeSeconds_ += deltaSeconds;
        if (session_->IsPlayerVictorious() && !wasRewardGranted_)
        {
            GrantVictoryReward();
        }

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
        screen.Box(5, 4, 98, 30, Color::BrightBlack);
        screen.CenterText(
            6,
            LocalizedText::GetBossName(context_.language, boss_.type),
            Color::BrightRed);

        context_.hudRenderer.DrawProgressBar(
            screen,
            12,
            9,
            28,
            static_cast<float>(session_->GetPlayerHealth()) / 100.0f,
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

        // 첫 버전은 보스 형상보다 타이밍 전투의 상태와 입력 결과를 명확히 보여준다.
        screen.CenterText(13, L"       ╱╲       ", Color::BrightRed);
        screen.CenterText(14, L"   ╭──╯  ╰──╮   ", Color::BrightRed);
        screen.CenterText(15, L"   │  ◆  ◆  │   ", Color::BrightYellow);
        screen.CenterText(16, L"   ╰──╮  ╭──╯   ", Color::BrightRed);
        screen.CenterText(17, L"      ╰──╯      ", Color::BrightBlack);

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

        // 마지막 공격 피해를 유지해 타이밍 입력이 실제 수치에 미친 영향을 읽을 수 있게 한다.
        if (lastDamage_ > 0)
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
                    L"[ SPACE ] 중앙에 맞춰 공격   [ ESC ] 후퇴",
                    L"[ SPACE ] STRIKE THE CENTER   [ ESC ] RETREAT"),
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
        session_.reset();
    }

    void BattleScene::GrantVictoryReward()
    {
        // 진행도 해금과 보상을 같은 완료 지점에서 반영해 일부만 지급되는 상태를 막는다.
        context_.progress.GrantGold(boss_.rewardGold);
        context_.progress.GrantFragments(boss_.rewardFragments);
        context_.progress.RecordBossVictory();
        wasRewardGranted_ = true;
    }
}
