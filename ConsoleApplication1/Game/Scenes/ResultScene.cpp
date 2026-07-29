#include "Game/Scenes/ResultScene.h"

#include "Core/IRandomProvider.h"
#include "Game/Domain/ForgeOutcome.h"
#include "Game/Domain/PlayerProgress.h"
#include "Game/Effects/ParticleSystem.h"
#include "Game/Scenes/GameHudRenderer.h"
#include "Game/Scenes/SceneContext.h"
#include "Platform/IInput.h"
#include "Rendering/IScreen.h"

#include <cassert>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace ss
{
    ResultScene::ResultScene(SceneContext& context)
        : context_(context)
    {
    }

    void ResultScene::OnEnter()
    {
        assert(context_.lastOutcome.has_value());
        sceneTimeSeconds_ = 0.0f;
    }

    SceneTransition ResultScene::Update(float deltaSeconds)
    {
        sceneTimeSeconds_ += deltaSeconds;

        const ForgeOutcome& outcome = *context_.lastOutcome;
        const Color swordColor = context_.hudRenderer.GetSwordColor(outcome.newLevel);
        if (context_.randomProvider.NextFloat(0.0f, 1.0f) < 0.22f)
        {
            context_.particles.SpawnResultParticle(outcome.succeeded, swordColor);
        }

        if (context_.input.WasPressed(InputKey::Escape))
        {
            return SceneTransition::To(SceneType::Forge);
        }

        const bool isReturnRequested =
            context_.input.WasPressed(InputKey::Enter) ||
            context_.input.WasPressed(InputKey::Space);
        if (sceneTimeSeconds_ > 0.65f && isReturnRequested)
        {
            return SceneTransition::To(SceneType::Forge);
        }
        return SceneTransition::None();
    }

    void ResultScene::Render(IScreen& screen) const
    {
        assert(context_.lastOutcome.has_value());
        const ForgeOutcome& outcome = *context_.lastOutcome;

        context_.hudRenderer.DrawBackdrop(screen, context_.worldTimeSeconds);
        context_.hudRenderer.DrawHeader(screen, context_.progress);

        const Color swordColor = context_.hudRenderer.GetSwordColor(outcome.newLevel);
        const Color resultColor = outcome.succeeded ? swordColor : Color::BrightRed;
        screen.Box(
            12,
            5,
            91,
            29,
            sceneTimeSeconds_ < 0.18f ? Color::BrightWhite : resultColor);
        screen.CenterText(7, GetHeadline(outcome), resultColor);
        screen.CenterText(
            9,
            GetDetail(outcome),
            outcome.succeeded ? Color::BrightWhite : Color::BrightBlack);

        context_.hudRenderer.DrawSword(
            screen,
            52,
            11,
            outcome.succeeded ? swordColor : Color::BrightBlack,
            outcome.newLevel,
            (std::sin(context_.worldTimeSeconds * 5.0f) + 1.0f) * 0.5f,
            context_.worldTimeSeconds);

        std::wstringstream transition;
        transition << L"+" << outcome.previousLevel
                   << (outcome.succeeded ? L"  ▶▶▶  +" : L"  ───  +")
                   << outcome.newLevel;
        screen.CenterText(26, transition.str(), resultColor);

        std::wstringstream score;
        score << L"CRAFT "
              << std::fixed
              << std::setprecision(0)
              << outcome.craftScore * 100.0f
              << L"%     FINAL RESONANCE "
              << outcome.finalChance * 100.0f
              << L"%";
        screen.CenterText(28, score.str(), Color::BrightBlack);

        const bool isPromptVisible =
            sceneTimeSeconds_ > 0.65f &&
            static_cast<int>(context_.worldTimeSeconds * 2.0f) % 2 == 0;
        if (isPromptVisible)
        {
            screen.CenterText(
                31,
                L"[ ENTER ]  RETURN TO THE ANVIL",
                Color::BrightYellow);
        }
    }

    void ResultScene::OnExit()
    {
    }

    std::wstring_view ResultScene::GetHeadline(const ForgeOutcome& outcome) noexcept
    {
        if (outcome.succeeded)
        {
            return outcome.wasCritical
                ? L"RESONANCE : PERFECT ASCENSION"
                : L"THE BLADE ANSWERS";
        }

        switch (outcome.failureConsequence)
        {
        case FailureConsequence::LevelMaintained:
            return L"THE ECHO FADES";
        case FailureConsequence::FragmentConsumed:
            return L"MEMORY SHARD SHATTERED";
        case FailureConsequence::LevelLost:
            return L"A MEMORY WAS LOST";
        case FailureConsequence::None:
            break;
        }
        return L"THE RITUAL ENDS";
    }

    std::wstring_view ResultScene::GetDetail(const ForgeOutcome& outcome) noexcept
    {
        if (outcome.wasCritical)
        {
            return L"A flawless rhythm awakened two memories at once.";
        }
        if (outcome.succeeded)
        {
            return L"Steel, flame, and will have become one.";
        }

        switch (outcome.failureConsequence)
        {
        case FailureConsequence::LevelMaintained:
            return L"The blade endured. Its enhancement remains unchanged.";
        case FailureConsequence::FragmentConsumed:
            return L"A shard sacrificed itself to protect the enhancement.";
        case FailureConsequence::LevelLost:
            return L"The blade survives, but one enhancement has faded.";
        case FailureConsequence::None:
            break;
        }
        return L"The forge has fallen silent.";
    }
}
