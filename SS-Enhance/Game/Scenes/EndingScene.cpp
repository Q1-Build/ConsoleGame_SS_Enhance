#include "Game/Scenes/EndingScene.h"

#include "Game/Domain/PlayerProgress.h"
#include "Game/Effects/ParticleSystem.h"
#include "Game/Scenes/GameHudRenderer.h"
#include "Game/Scenes/LocalizedText.h"
#include "Game/Scenes/SceneContext.h"
#include "Platform/IInput.h"
#include "Rendering/IScreen.h"

#include <sstream>

namespace ss
{
    EndingScene::EndingScene(SceneContext& context)
        : context_(context)
    {
    }

    void EndingScene::OnEnter()
    {
        // 전투 파티클을 제거해 엔딩의 정적인 검 연출과 시각적으로 분리한다.
        sceneTimeSeconds_ = 0.0f;
        context_.particles.Clear();
    }

    SceneTransition EndingScene::Update(float deltaSeconds)
    {
        sceneTimeSeconds_ += deltaSeconds;

        // 최종 보스의 공격 입력이 즉시 종료 입력으로 이어지지 않도록 짧은 읽기 시간을 보장한다.
        if (sceneTimeSeconds_ > 0.8f &&
            (context_.input.WasPressed(InputKey::Enter) ||
             context_.input.WasPressed(InputKey::Space) ||
             context_.input.WasPressed(InputKey::Escape)))
        {
            return SceneTransition::To(SceneType::Exit);
        }
        return SceneTransition::None();
    }

    void EndingScene::Render(IScreen& screen) const
    {
        context_.hudRenderer.DrawBackdrop(screen, context_.worldTimeSeconds);
        screen.Box(12, 4, 91, 30, Color::BrightMagenta);
        screen.CenterText(
            7,
            LocalizedText::Select(
                context_.language,
                L"별을 삼키는 자",
                L"STAR EATER"),
            Color::BrightMagenta);
        screen.CenterText(
            10,
            LocalizedText::Select(
                context_.language,
                L"마지막 기억이 검에 새겨졌습니다.",
                L"THE FINAL MEMORY HAS BEEN FORGED."),
            Color::BrightWhite);

        context_.hudRenderer.DrawSword(
            screen,
            31,
            11,
            Color::BrightMagenta,
            12,
            1.0f,
            context_.worldTimeSeconds);

        // 검과 기록을 좌우로 나눠 긴 현지화 문구가 검 형상을 덮지 않게 한다.
        std::wstringstream attempts;
        attempts << LocalizedText::Select(
                        context_.language, L"강화 시도 ", L"ATTEMPTS ")
                 << context_.progress.GetAttemptCount();
        screen.Text(55, 16, attempts.str(), Color::BrightCyan);

        std::wstringstream successes;
        successes << LocalizedText::Select(
                          context_.language, L"강화 성공 ", L"SUCCESSES ")
                  << context_.progress.GetSuccessCount();
        screen.Text(55, 19, successes.str(), Color::BrightCyan);

        std::wstringstream bosses;
        bosses << LocalizedText::Select(
                      context_.language, L"보스 격파 ", L"BOSSES ")
               << context_.progress.GetBossVictoryCount();
        screen.Text(55, 22, bosses.str(), Color::BrightCyan);
        screen.Text(
            55,
            25,
            LocalizedText::GetDifficultyName(
                context_.language,
                context_.difficulty),
            Color::BrightYellow);
        screen.CenterTextIn(
            50,
            88,
            28,
            LocalizedText::Select(
                context_.language,
                L"[ ENTER ] 여정 마치기",
                L"[ ENTER ] END THE JOURNEY"),
            Color::BrightYellow);
    }

    void EndingScene::OnExit()
    {
    }
}
