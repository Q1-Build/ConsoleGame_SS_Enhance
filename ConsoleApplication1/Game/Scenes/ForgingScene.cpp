#include "Game/Scenes/ForgingScene.h"

#include "Core/IRandomProvider.h"
#include "Game/Domain/ForgeRules.h"
#include "Game/Domain/ForgeSession.h"
#include "Game/Domain/PlayerProgress.h"
#include "Game/Effects/ParticleSystem.h"
#include "Game/Scenes/GameHudRenderer.h"
#include "Game/Scenes/SceneContext.h"
#include "Platform/IInput.h"
#include "Rendering/IScreen.h"

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <sstream>

namespace ss
{
    ForgingScene::ForgingScene(SceneContext& context)
        : context_(context)
    {
    }

    ForgingScene::~ForgingScene() = default;

    void ForgingScene::OnEnter()
    {
        const int swordLevel = context_.progress.GetSword().GetLevel();
        session_ = std::make_unique<ForgeSession>(swordLevel);
        context_.particles.Clear();
    }

    SceneTransition ForgingScene::Update(float deltaSeconds)
    {
        assert(session_ != nullptr);

        if (context_.input.WasPressed(InputKey::Escape))
        {
            return SceneTransition::To(SceneType::Forge);
        }

        const bool isCooling =
            context_.input.IsDown(InputKey::A) ||
            context_.input.IsDown(InputKey::Left);
        const bool isStoking =
            context_.input.IsDown(InputKey::D) ||
            context_.input.IsDown(InputKey::Right);
        session_->Update(
            deltaSeconds,
            context_.worldTimeSeconds,
            isCooling,
            isStoking);

        const bool isStrikeRequested =
            context_.input.WasPressed(InputKey::Space) ||
            context_.input.WasPressed(InputKey::Enter);
        if (isStrikeRequested)
        {
            const std::optional<float> score = session_->TryStrike();
            if (score.has_value())
            {
                context_.particles.SpawnImpact(*score);
            }
        }

        if (session_->IsComplete())
        {
            return ResolveForge();
        }
        return SceneTransition::None();
    }

    void ForgingScene::Render(IScreen& screen) const
    {
        assert(session_ != nullptr);

        context_.hudRenderer.DrawBackdrop(screen, context_.worldTimeSeconds);
        context_.hudRenderer.DrawHeader(screen, context_.progress);

        const Color frameColor = session_->GetImpactFlash() > 0.0f
            ? Color::BrightWhite
            : Color::BrightBlack;
        screen.Box(4, 5, 99, 29, frameColor);
        screen.CenterText(
            6,
            L"—  LISTEN TO THE HAMMER'S ECHO  —",
            Color::BrightRed);

        const float hammerSwing = session_->GetStrikeCooldown() > 0.18f
            ? session_->GetStrikeCooldown() * 7.0f
            : 0.0f;
        const int hammerX = 39 - static_cast<int>(hammerSwing * 3.0f);
        const int hammerY = 12 - static_cast<int>(hammerSwing * 2.0f);
        const Color hammerColor = session_->GetImpactFlash() > 0.0f
            ? Color::BrightWhite
            : Color::BrightBlack;
        screen.Text(hammerX, hammerY, L"██████", hammerColor);
        screen.Text(
            hammerX + 2,
            hammerY + 1,
            L"██",
            session_->GetImpactFlash() > 0.0f
                ? Color::BrightYellow
                : Color::BrightBlack);
        screen.Line(
            hammerX + 3,
            hammerY + 2,
            50,
            16,
            L'╲',
            session_->GetImpactFlash() > 0.0f
                ? Color::BrightYellow
                : Color::BrightBlack);

        const float heat = session_->GetHeat();
        const Color bladeColor = heat > 82.0f
            ? Color::BrightWhite
            : (heat > 58.0f
                ? Color::BrightYellow
                : (heat > 28.0f ? Color::BrightRed : Color::BrightBlack));
        screen.Text(40, 16, L"═════════════════════════►", bladeColor);
        screen.Text(
            34,
            17,
            L"▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄",
            Color::BrightBlack);
        screen.Text(
            38,
            18,
            L"▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀",
            Color::BrightBlack);

        context_.hudRenderer.DrawProgressBar(
            screen,
            10,
            21,
            31,
            heat / 100.0f,
            bladeColor,
            L"HEAT   ");
        const bool isResonantHeat = heat >= 58.0f && heat <= 78.0f;
        screen.Text(
            71,
            21,
            isResonantHeat ? L"RESONANT" : L"UNSTABLE",
            isResonantHeat ? Color::BrightGreen : Color::BrightRed);

        screen.Text(10, 24, L"RHYTHM ", Color::BrightWhite);
        screen.Put(18, 24, L'[', Color::BrightBlack);
        constexpr int kRhythmWidth = 62;
        const int markerPosition = static_cast<int>(
            session_->GetMarker() * static_cast<float>(kRhythmWidth - 1));
        for (int index = 0; index < kRhythmWidth; ++index)
        {
            const bool isSweetSpot = index >= 27 && index <= 34;
            screen.Put(
                19 + index,
                24,
                isSweetSpot ? L'◆' : L'─',
                isSweetSpot ? Color::BrightGreen : Color::BrightBlack);
        }
        screen.Put(19 + markerPosition, 24, L'█', Color::BrightYellow);
        screen.Put(19 + kRhythmWidth, 24, L']', Color::BrightBlack);

        std::wstringstream timer;
        timer << L"TIME "
              << std::fixed
              << std::setprecision(1)
              << std::max(0.0f, session_->GetTimeLeft())
              << L"s";
        screen.Text(
            10,
            27,
            timer.str(),
            session_->GetTimeLeft() < 4.0f
                ? Color::BrightRed
                : Color::BrightWhite);

        screen.Text(44, 27, L"STRIKES", Color::BrightBlack);
        const std::vector<float>& scores = session_->GetStrikeScores();
        for (int index = 0; index < 3; ++index)
        {
            if (index >= static_cast<int>(scores.size()))
            {
                screen.Text(53 + index * 9, 27, L"[ -- ]", Color::BrightBlack);
                continue;
            }

            const float score = scores[static_cast<std::size_t>(index)];
            const std::wstring_view text = score >= 0.86f
                ? L"[PERF]"
                : (score >= 0.62f ? L"[GOOD]" : L"[MISS]");
            const Color color = score >= 0.86f
                ? Color::BrightCyan
                : (score >= 0.62f ? Color::BrightYellow : Color::BrightRed);
            screen.Text(53 + index * 9, 27, text, color);
        }

        screen.CenterText(
            31,
            L"A / D : TEMPER HEAT     SPACE : STRIKE     ESC : ABORT",
            Color::BrightBlack);
        if (session_->GetImpactFlash() > 0.0f)
        {
            screen.CenterText(10, L"✦  K R A A A N G  ✦", Color::BrightYellow);
        }
    }

    void ForgingScene::OnExit()
    {
        session_.reset();
    }

    SceneTransition ForgingScene::ResolveForge()
    {
        assert(session_ != nullptr);

        const float randomRoll = context_.randomProvider.NextFloat(0.0f, 1.0f);
        context_.lastOutcome = context_.forgeRules.Resolve(
            context_.progress,
            session_->GetStrikeScores(),
            randomRoll);

        const ForgeOutcome& outcome = *context_.lastOutcome;
        const Color swordColor = context_.hudRenderer.GetSwordColor(outcome.newLevel);
        context_.particles.Clear();
        context_.particles.SpawnResultBurst(
            outcome.succeeded,
            swordColor,
            90);
        return SceneTransition::To(SceneType::Result);
    }
}
