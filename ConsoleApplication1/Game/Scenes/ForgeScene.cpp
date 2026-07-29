#include "Game/Scenes/ForgeScene.h"

#include "Core/IRandomProvider.h"
#include "Game/Domain/ForgeRules.h"
#include "Game/Domain/PlayerProgress.h"
#include "Game/Effects/ParticleSystem.h"
#include "Game/Scenes/GameHudRenderer.h"
#include "Game/Scenes/SceneContext.h"
#include "Platform/IInput.h"
#include "Rendering/IScreen.h"

#include <cmath>
#include <iomanip>
#include <sstream>

namespace ss
{
    ForgeScene::ForgeScene(SceneContext& context)
        : context_(context)
    {
    }

    void ForgeScene::OnEnter()
    {
        context_.notice.clear();
    }

    SceneTransition ForgeScene::Update(float deltaSeconds)
    {
        static_cast<void>(deltaSeconds);

        if (context_.randomProvider.NextFloat(0.0f, 1.0f) < 0.08f)
        {
            context_.particles.SpawnAmbientEmber();
        }

        if (context_.input.WasPressed(InputKey::Escape) ||
            context_.input.WasPressed(InputKey::Q))
        {
            return SceneTransition::To(SceneType::Exit);
        }

        const bool isStartRequested =
            context_.input.WasPressed(InputKey::Enter) ||
            context_.input.WasPressed(InputKey::Space);
        if (!isStartRequested)
        {
            return SceneTransition::None();
        }

        const int swordLevel = context_.progress.GetSword().GetLevel();
        const int forgeCost = ForgeRules::CalculateCost(swordLevel);
        if (!context_.progress.CanAfford(forgeCost))
        {
            context_.progress.GrantGold(600);
            context_.notice =
                L"Not enough gold. The guild grants an emergency contract: +600 G";
            return SceneTransition::None();
        }

        const bool wasPaid = context_.progress.SpendGold(forgeCost);
        if (!wasPaid)
        {
            return SceneTransition::None();
        }

        context_.progress.RecordAttempt();
        context_.particles.Clear();
        return SceneTransition::To(SceneType::Forging);
    }

    void ForgeScene::Render(IScreen& screen) const
    {
        context_.hudRenderer.DrawBackdrop(screen, context_.worldTimeSeconds);
        context_.hudRenderer.DrawHeader(screen, context_.progress);

        screen.Box(4, 5, 65, 29, Color::BrightBlack);
        screen.Box(68, 5, 99, 29, Color::BrightBlack);
        screen.Text(7, 6, L"THE ANVIL", Color::BrightRed);
        screen.Text(71, 6, L"BLADE MEMORY", Color::BrightCyan);

        const Sword& sword = context_.progress.GetSword();
        const Color swordColor = context_.hudRenderer.GetSwordColor(sword.GetLevel());
        context_.hudRenderer.DrawSword(
            screen,
            84,
            8,
            swordColor,
            sword.GetLevel(),
            (std::sin(context_.worldTimeSeconds * 2.8f) + 1.0f) * 0.5f,
            context_.worldTimeSeconds);

        std::wstringstream level;
        level << L"+" << sword.GetLevel() << L"  " << sword.GetName();
        screen.CenterText(26, level.str(), swordColor);
        screen.CenterText(27, L"Every scar becomes a story.", Color::BrightBlack);

        screen.Text(8, 9, L"ENHANCEMENT RITUAL", Color::BrightWhite);
        screen.Text(
            8,
            11,
            L"Control the flame. Read the rhythm. Strike three times.",
            Color::BrightBlack);
        screen.Text(8, 13, L"← / A", Color::BrightCyan);
        screen.Text(20, 13, L"cool the steel", Color::White);
        screen.Text(8, 15, L"→ / D", Color::BrightRed);
        screen.Text(20, 15, L"feed the flame", Color::White);
        screen.Text(8, 17, L"SPACE", Color::BrightYellow);
        screen.Text(20, 17, L"strike when rhythm and heat align", Color::White);

        std::wstringstream chance;
        chance << L"Base resonance    "
               << std::fixed
               << std::setprecision(0)
               << ForgeRules::GetBaseChance(sword.GetLevel()) * 100.0f
               << L"%";
        screen.Text(8, 21, chance.str(), Color::BrightCyan);

        const int forgeCost = ForgeRules::CalculateCost(sword.GetLevel());
        std::wstringstream cost;
        cost << L"Ritual cost       " << forgeCost << L" G";
        const Color costColor = context_.progress.CanAfford(forgeCost)
            ? Color::BrightYellow
            : Color::BrightRed;
        screen.Text(8, 23, cost.str(), costColor);

        screen.Text(8, 26, L"[ ENTER / SPACE ] BEGIN RITUAL", Color::BrightYellow);
        screen.Text(8, 27, L"[ Q / ESC ]       LEAVE FORGE", Color::BrightBlack);

        if (!context_.notice.empty())
        {
            screen.CenterText(31, context_.notice, Color::BrightYellow);
            return;
        }

        std::wstringstream record;
        record << L"RITUALS " << context_.progress.GetAttemptCount()
               << L"   ASCENSIONS " << context_.progress.GetSuccessCount();
        screen.CenterText(31, record.str(), Color::BrightBlack);
    }

    void ForgeScene::OnExit()
    {
    }
}
