#include "Game/Scenes/ResultScene.h"

#include "Game/Domain/ForgeOutcome.h"
#include "Game/Domain/PlayerProgress.h"
#include "Game/Effects/ParticleSystem.h"
#include "Game/Scenes/GameHudRenderer.h"
#include "Game/Scenes/LocalizedText.h"
#include "Game/Scenes/SceneContext.h"
#include "Platform/IAudio.h"
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
        context_.audio.PlayMusic(MusicTrack::Forge);
        const ForgeOutcome& outcome = *context_.lastOutcome;
        context_.audio.PlaySound(
            outcome.wasCritical
                ? SoundEffect::ForgeCritical
                : (outcome.succeeded
                    ? SoundEffect::ForgeSuccess
                    : SoundEffect::ForgeFailure));
    }

    SceneTransition ResultScene::Update(float deltaSeconds)
    {
        sceneTimeSeconds_ += deltaSeconds;

        const ForgeOutcome& outcome = *context_.lastOutcome;
        const Color swordColor = context_.hudRenderer.GetSwordColor(outcome.newLevel);

        // 초기 폭발 뒤에도 작은 결과 파티클을 추가해 정지 화면처럼 보이지 않게 한다.
        constexpr float kResultParticlesPerSecond = 13.0f;
        context_.particles.EmitResultParticles(
            deltaSeconds,
            kResultParticlesPerSecond,
            outcome.succeeded,
            swordColor);

        if (context_.input.WasPressed(InputKey::Escape))
        {
            context_.audio.StopSounds();
            context_.audio.PlaySound(SoundEffect::MenuBack);
            return SceneTransition::To(SceneType::Forge);
        }

        // 결과를 읽을 최소 시간을 보장해 타격 키가 복귀 입력으로 연속 처리되지 않게 한다.
        const bool isReturnRequested =
            context_.input.WasPressed(InputKey::Enter) ||
            context_.input.WasPressed(InputKey::Space);
        if (sceneTimeSeconds_ > 0.65f && isReturnRequested)
        {
            context_.audio.StopSounds();
            context_.audio.PlaySound(SoundEffect::MenuConfirm);
            return SceneTransition::To(SceneType::Forge);
        }
        return SceneTransition::None();
    }

    void ResultScene::Render(IScreen& screen) const
    {
        assert(context_.lastOutcome.has_value());
        const ForgeOutcome& outcome = *context_.lastOutcome;

        context_.hudRenderer.DrawBackdrop(screen, context_.worldTimeSeconds);
        context_.hudRenderer.DrawHeader(
            screen,
            context_.progress,
            context_.language,
            context_.difficulty);

        const Color swordColor = context_.hudRenderer.GetSwordColor(outcome.newLevel);
        const Color resultColor = outcome.succeeded ? swordColor : Color::BrightRed;

        // 장면 진입 직후 잠깐 흰색 테두리를 사용해 강화 판정 순간을 강조한다.
        screen.Box(
            12,
            5,
            91,
            29,
            sceneTimeSeconds_ < 0.18f ? Color::BrightWhite : resultColor);
        screen.CenterText(
            7,
            GetHeadline(outcome, context_.language),
            resultColor);
        screen.CenterText(
            9,
            GetDetail(outcome, context_.language),
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
        score << LocalizedText::Select(
                     context_.language, L"제련도 ", L"CRAFT ")
              << std::fixed
              << std::setprecision(0)
              << outcome.craftScore * 100.0f
              << LocalizedText::Select(
                     context_.language, L"%     최종 공명률 ", L"%     FINAL RESONANCE ")
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
                LocalizedText::Select(
                    context_.language,
                    L"[ ENTER ]  모루로 돌아가기",
                    L"[ ENTER ]  RETURN TO THE ANVIL"),
                Color::BrightYellow);
        }
    }

    void ResultScene::OnExit()
    {
        // 결과 장면의 전달 값과 전용 효과가 다음 강화 화면에 섞이지 않게 수명을 함께 끝낸다.
        context_.lastOutcome.reset();
        context_.particles.Clear();
    }

    std::wstring_view ResultScene::GetHeadline(
        const ForgeOutcome& outcome,
        Language language) noexcept
    {
        // 도메인 결과를 사용자에게 보여줄 문구로 변환하는 책임은 장면에만 둔다.
        if (outcome.succeeded)
        {
            return outcome.wasCritical
                ? LocalizedText::Select(
                    language, L"공명 : 완벽한 승급", L"RESONANCE : PERFECT ASCENSION")
                : LocalizedText::Select(
                    language, L"검이 응답했다", L"THE BLADE ANSWERS");
        }

        switch (outcome.failureConsequence)
        {
        case FailureConsequence::LevelMaintained:
            return LocalizedText::Select(language, L"메아리가 사라진다", L"THE ECHO FADES");
        case FailureConsequence::FragmentConsumed:
            return LocalizedText::Select(
                language, L"기억 조각이 부서졌다", L"MEMORY SHARD SHATTERED");
        case FailureConsequence::LevelLost:
            return LocalizedText::Select(
                language, L"하나의 기억을 잃었다", L"A MEMORY WAS LOST");
        case FailureConsequence::None:
            break;
        }
        return LocalizedText::Select(language, L"의식이 끝났다", L"THE RITUAL ENDS");
    }

    std::wstring_view ResultScene::GetDetail(
        const ForgeOutcome& outcome,
        Language language) noexcept
    {
        if (outcome.wasCritical)
        {
            return LocalizedText::Select(
                language,
                L"완벽한 리듬이 두 기억을 동시에 깨웠습니다.",
                L"A flawless rhythm awakened two memories at once.");
        }
        if (outcome.succeeded)
        {
            return LocalizedText::Select(
                language,
                L"강철과 불꽃, 의지가 하나가 되었습니다.",
                L"Steel, flame, and will have become one.");
        }

        switch (outcome.failureConsequence)
        {
        case FailureConsequence::LevelMaintained:
            return LocalizedText::Select(
                language,
                L"검이 버텼습니다. 강화 단계는 유지됩니다.",
                L"The blade endured. Its enhancement remains unchanged.");
        case FailureConsequence::FragmentConsumed:
            return LocalizedText::Select(
                language,
                L"기억 조각이 강화를 지키기 위해 희생되었습니다.",
                L"A shard sacrificed itself to protect the enhancement.");
        case FailureConsequence::LevelLost:
            return LocalizedText::Select(
                language,
                L"검은 살아남았지만 강화 한 단계가 사라졌습니다.",
                L"The blade survives, but one enhancement has faded.");
        case FailureConsequence::None:
            break;
        }
        return LocalizedText::Select(
            language, L"대장간이 고요해졌습니다.", L"The forge has fallen silent.");
    }
}
