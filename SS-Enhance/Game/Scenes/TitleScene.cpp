#include "Game/Scenes/TitleScene.h"

#include "Core/GameConstants.h"
#include "Core/IRandomProvider.h"
#include "Game/Effects/ParticleSystem.h"
#include "Game/Scenes/GameHudRenderer.h"
#include "Game/Scenes/LocalizedText.h"
#include "Game/Scenes/SceneContext.h"
#include "Platform/IInput.h"
#include "Rendering/IScreen.h"

#include <cmath>

namespace ss
{
    TitleScene::TitleScene(SceneContext& context)
        : context_(context)
    {
    }

    void TitleScene::OnEnter()
    {
        context_.notice.clear();
    }

    SceneTransition TitleScene::Update(float deltaSeconds)
    {
        static_cast<void>(deltaSeconds);

        // 프레임마다 낮은 확률로 생성해 시간에 따라 불규칙한 배경 불씨를 만든다.
        if (context_.randomProvider.NextFloat(0.0f, 1.0f) < 0.13f)
        {
            context_.particles.SpawnAmbientEmber();
        }

        if (context_.input.WasPressed(InputKey::Escape))
        {
            return SceneTransition::To(SceneType::Exit);
        }
        if (context_.input.WasPressed(InputKey::Left) ||
            context_.input.WasPressed(InputKey::A))
        {
            context_.language = Language::Korean;
        }
        if (context_.input.WasPressed(InputKey::Right) ||
            context_.input.WasPressed(InputKey::D))
        {
            context_.language = Language::English;
        }
        if (context_.input.WasPressed(InputKey::Enter) ||
            context_.input.WasPressed(InputKey::Space))
        {
            return SceneTransition::To(SceneType::Forge);
        }
        return SceneTransition::None();
    }

    void TitleScene::Render(IScreen& screen) const
    {
        context_.hudRenderer.DrawBackdrop(screen, context_.worldTimeSeconds);

        // 제목 색상을 주기적으로 교차해 정적인 문자 로고에 맥동감을 준다.
        const Color titleColor = std::sin(context_.worldTimeSeconds * 2.2f) > 0.0f
            ? Color::BrightRed
            : Color::BrightYellow;
        screen.CenterText(5, L"███████╗ ███████╗", titleColor);
        screen.CenterText(6, L"██╔════╝ ██╔════╝", titleColor);
        screen.CenterText(7, L"███████╗ ███████╗", titleColor);
        screen.CenterText(8, L"╚════██║ ╚════██║", titleColor);
        screen.CenterText(9, L"███████║ ███████║", titleColor);
        screen.CenterText(10, L"╚══════╝ ╚══════╝", titleColor);
        screen.CenterText(12, L"E  N  H  A  N  C  E", Color::BrightWhite);
        screen.CenterText(
            14,
            LocalizedText::Select(
                context_.language,
                L"—  검 은   모 든   것 을   기 억 한 다  —",
                L"—  T H E   S W O R D   R E M E M B E R S  —"),
            Color::BrightBlack);

        context_.hudRenderer.DrawSword(
            screen,
            kScreenWidth / 2,
            16,
            Color::BrightRed,
            6,
            (std::sin(context_.worldTimeSeconds * 3.0f) + 1.0f) * 0.5f,
            context_.worldTimeSeconds);

        // 입력 안내는 일정 주기로 점멸하지만 입력 가능 여부에는 영향을 주지 않는다.
        const Color promptColor =
            static_cast<int>(context_.worldTimeSeconds * 2.0f) % 2 == 0
                ? Color::BrightYellow
                : Color::White;
        screen.CenterText(
            29,
            LocalizedText::Select(
                context_.language,
                L"언어  ◀  한국어  |  ENGLISH  ▶",
                L"LANGUAGE  ◀  KOREAN  |  ENGLISH  ▶"),
            Color::BrightCyan);
        screen.CenterText(
            31,
            LocalizedText::Select(
                context_.language,
                L"[ ENTER ]  대장간 깨우기",
                L"[ ENTER ]  AWAKEN THE FORGE"),
            promptColor);
    }

    void TitleScene::OnExit()
    {
    }
}
