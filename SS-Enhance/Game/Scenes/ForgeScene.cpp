#include "Game/Scenes/ForgeScene.h"

#include "Core/IRandomProvider.h"
#include "Game/Domain/ForgeRules.h"
#include "Game/Domain/PlayerProgress.h"
#include "Game/Effects/ParticleSystem.h"
#include "Game/Scenes/GameHudRenderer.h"
#include "Game/Scenes/LocalizedText.h"
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
        const int forgeCost = ForgeRules::CalculateCost(
            swordLevel,
            context_.difficulty);
        if (!context_.progress.CanAfford(forgeCost))
        {
            // 플레이가 재화 부족으로 영구 중단되지 않도록 같은 장면에서 긴급 계약을 지급한다.
            context_.progress.GrantGold(600);
            context_.notice = std::wstring(LocalizedText::Select(
                context_.language,
                L"골드가 부족합니다. 길드의 긴급 계약 지원: +600 G",
                L"Not enough gold. The guild grants an emergency contract: +600 G"));
            return SceneTransition::None();
        }

        // 장면 전환 전에 비용과 시도 횟수를 확정해 중복 입력으로 두 번 시작되는 것을 막는다.
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
        context_.hudRenderer.DrawHeader(
            screen,
            context_.progress,
            context_.language);

        // 왼쪽은 조작과 확률, 오른쪽은 현재 검의 시각 정보로 영역을 구분한다.
        screen.Box(4, 5, 65, 29, Color::BrightBlack);
        screen.Box(68, 5, 99, 29, Color::BrightBlack);
        screen.Text(7, 6, LocalizedText::Select(
            context_.language, L"모루", L"THE ANVIL"), Color::BrightRed);
        screen.Text(71, 6, LocalizedText::Select(
            context_.language, L"검의 기억", L"BLADE MEMORY"), Color::BrightCyan);

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
        level << L"+" << sword.GetLevel() << L"  "
              << LocalizedText::GetSwordName(context_.language, sword.GetTier());
        screen.CenterTextIn(69, 98, 26, level.str(), swordColor);
        screen.CenterTextIn(69, 98, 27, LocalizedText::Select(
            context_.language,
            L"흔적마다 이야기가 깃든다.",
            L"Every scar becomes a story."), Color::BrightBlack);

        screen.Text(8, 9, LocalizedText::Select(
            context_.language, L"강화 의식", L"ENHANCEMENT RITUAL"), Color::BrightWhite);
        screen.Text(
            8,
            11,
            LocalizedText::Select(
                context_.language,
                L"불꽃을 조절하고, 박자에 맞춰 세 번 타격하세요.",
                L"Control the flame. Read the rhythm. Strike three times."),
            Color::BrightBlack);
        screen.Text(8, 13, L"← / A", Color::BrightCyan);
        screen.Text(20, 13, LocalizedText::Select(
            context_.language, L"검을 식힌다", L"cool the steel"), Color::White);
        screen.Text(8, 15, L"→ / D", Color::BrightRed);
        screen.Text(20, 15, LocalizedText::Select(
            context_.language, L"화력을 높인다", L"feed the flame"), Color::White);
        screen.Text(8, 17, L"SPACE", Color::BrightYellow);
        screen.Text(20, 17, LocalizedText::Select(
            context_.language,
            L"리듬과 온도가 맞을 때 타격",
            L"strike when rhythm and heat align"), Color::White);

        std::wstringstream chance;
        chance << LocalizedText::Select(
                      context_.language, L"기본 공명률        ", L"Base resonance    ")
               << std::fixed
               << std::setprecision(0)
               << ForgeRules::GetBaseChance(sword.GetLevel()) * 100.0f
               << L"%";
        screen.Text(8, 21, chance.str(), Color::BrightCyan);

        const int forgeCost = ForgeRules::CalculateCost(
            sword.GetLevel(),
            context_.difficulty);
        std::wstringstream cost;
        cost << LocalizedText::Select(
                    context_.language, L"의식 비용          ", L"Ritual cost       ")
             << forgeCost << L" G";
        const Color costColor = context_.progress.CanAfford(forgeCost)
            ? Color::BrightYellow
            : Color::BrightRed;
        screen.Text(8, 23, cost.str(), costColor);

        screen.Text(8, 26, LocalizedText::Select(
            context_.language,
            L"[ ENTER / SPACE ] 강화 시작",
            L"[ ENTER / SPACE ] BEGIN RITUAL"), Color::BrightYellow);
        screen.Text(8, 27, LocalizedText::Select(
            context_.language,
            L"[ Q / ESC ]       게임 종료",
            L"[ Q / ESC ]       LEAVE FORGE"), Color::BrightBlack);

        if (!context_.notice.empty())
        {
            screen.CenterText(31, context_.notice, Color::BrightYellow);
            return;
        }

        std::wstringstream record;
        record << LocalizedText::Select(context_.language, L"시도 ", L"RITUALS ")
               << context_.progress.GetAttemptCount()
               << LocalizedText::Select(context_.language, L"   성공 ", L"   ASCENSIONS ")
               << context_.progress.GetSuccessCount();
        screen.CenterText(31, record.str(), Color::BrightBlack);
    }

    void ForgeScene::OnExit()
    {
    }
}
