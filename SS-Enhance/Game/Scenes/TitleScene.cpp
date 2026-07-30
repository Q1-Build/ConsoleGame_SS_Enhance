#include "Game/Scenes/TitleScene.h"

#include "Core/GameConstants.h"
#include "Game/Effects/ParticleSystem.h"
#include "Game/Scenes/GameHudRenderer.h"
#include "Game/Scenes/LocalizedText.h"
#include "Game/Scenes/SceneContext.h"
#include "Platform/IAudio.h"
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
        context_.audio.PlayMusic(MusicTrack::Title);
    }

    SceneTransition TitleScene::Update(float deltaSeconds)
    {
        // 생성률 계산은 파티클 시스템에 맡겨 타이틀 연출 밀도가 프레임 속도에 좌우되지 않게 한다.
        constexpr float kAmbientEmbersPerSecond = 8.0f;
        context_.particles.EmitAmbientEmbers(
            deltaSeconds,
            kAmbientEmbersPerSecond);

        if (context_.input.WasPressed(InputKey::Escape))
        {
            context_.audio.PlaySound(SoundEffect::MenuBack);
            return SceneTransition::To(SceneType::Exit);
        }
        if (context_.input.WasPressed(InputKey::Enter) ||
            context_.input.WasPressed(InputKey::Space))
        {
            context_.audio.PlaySound(SoundEffect::MenuConfirm);
            return SceneTransition::To(SceneType::Settings);
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
        screen.CenterText(2, L"███████╗ ███████╗", titleColor);
        screen.CenterText(3, L"██╔════╝ ██╔════╝", titleColor);
        screen.CenterText(4, L"███████╗ ███████╗", titleColor);
        screen.CenterText(5, L"╚════██║ ╚════██║", titleColor);
        screen.CenterText(6, L"███████║ ███████║", titleColor);
        screen.CenterText(7, L"╚══════╝ ╚══════╝", titleColor);
        screen.CenterText(9, L"E  N  H  A  N  C  E", Color::BrightWhite);
        screen.CenterText(
            11,
            LocalizedText::Select(
                context_.language,
                L"—  검 은   모 든   것 을   기 억 한 다  —",
                L"—  T H E   B L A D E   R E M E M B E R S  —"),
            Color::BrightBlack);

        context_.hudRenderer.DrawSword(
            screen,
            kGameViewportWidth / 2,
            13,
            Color::BrightRed,
            6,
            (std::sin(context_.worldTimeSeconds * 3.0f) + 1.0f) * 0.5f,
            context_.worldTimeSeconds);

        // 입력 안내는 일정 주기로 점멸하지만 입력 가능 여부에는 영향을 주지 않는다.
        const Color promptColor =
            static_cast<int>(context_.worldTimeSeconds * 2.0f) % 2 == 0
                ? Color::BrightYellow
                : Color::White;
        // 검 손잡이 아래 두 행을 안내 전용 영역으로 남겨 검 형상과 문구가 겹치지 않게 한다.
        screen.CenterText(
            31,
            LocalizedText::Select(
                context_.language,
                L"언어, 볼륨과 난이도는 다음 화면에서 설정합니다.",
                L"LANGUAGE, VOLUME, AND DIFFICULTY ARE SET NEXT."),
            Color::BrightCyan);
        screen.CenterText(
            32,
            LocalizedText::Select(
                context_.language,
                L"[ ENTER ]  설정으로",
                L"[ ENTER ]  OPEN SETTINGS"),
            promptColor);
    }

    void TitleScene::OnExit()
    {
    }
}
