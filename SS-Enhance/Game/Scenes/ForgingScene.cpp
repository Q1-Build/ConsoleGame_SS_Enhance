#include "Game/Scenes/ForgingScene.h"

#include "Core/IRandomProvider.h"
#include "Game/Domain/ForgeRules.h"
#include "Game/Domain/ForgeSession.h"
#include "Game/Domain/PlayerProgress.h"
#include "Game/Effects/ParticleSystem.h"
#include "Game/Scenes/GameHudRenderer.h"
#include "Game/Scenes/LocalizedText.h"
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
        // 진입 시점의 검 단계로 난도를 고정해 제련 도중 외부 상태 변화에 영향받지 않게 한다.
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

        // 키보드 종류에 관계없이 문자 키와 방향키를 같은 도메인 입력으로 합친다.
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
                // 쿨다운 중 입력은 점수가 없으므로 실제 타격이 성립한 경우에만 효과를 생성한다.
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
        context_.hudRenderer.DrawHeader(
            screen,
            context_.progress,
            context_.language);

        const Color frameColor = session_->GetImpactFlash() > 0.0f
            ? Color::BrightWhite
            : Color::BrightBlack;

        // 충돌 직후 프레임과 망치를 밝게 바꿔 별도 이미지 없이 화면 플래시를 표현한다.
        screen.Box(4, 5, 99, 29, frameColor);
        screen.CenterText(
            6,
            LocalizedText::Select(
                context_.language,
                L"—  망치의 메아리에 귀 기울여라  —",
                L"—  LISTEN TO THE HAMMER'S ECHO  —"),
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

        // 현재 온도를 검신 색과 상태 문구에 동시에 반영해 수치를 보지 않아도 상태를 알 수 있게 한다.
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
            LocalizedText::Select(context_.language, L"온도   ", L"HEAT   "));
        const bool isResonantHeat = heat >= 58.0f && heat <= 78.0f;
        screen.Text(
            71,
            21,
            isResonantHeat
                ? LocalizedText::Select(context_.language, L"공명", L"RESONANT")
                : LocalizedText::Select(context_.language, L"불안정", L"UNSTABLE"),
            isResonantHeat ? Color::BrightGreen : Color::BrightRed);

        screen.Text(10, 24, LocalizedText::Select(
            context_.language, L"리듬   ", L"RHYTHM "), Color::BrightWhite);
        screen.Put(18, 24, L'[', Color::BrightBlack);
        constexpr int kRhythmWidth = 62;

        // 0~1 리듬 위치를 고정 폭 문자 좌표로 변환하고 중앙의 성공 구간과 겹쳐 표시한다.
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
        timer << LocalizedText::Select(context_.language, L"시간 ", L"TIME ")
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

        screen.Text(44, 27, LocalizedText::Select(
            context_.language, L"타격", L"STRIKES"), Color::BrightBlack);
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
                ? LocalizedText::Select(context_.language, L"[완벽]", L"[PERF]")
                : (score >= 0.62f
                    ? LocalizedText::Select(context_.language, L"[좋음]", L"[GOOD]")
                    : LocalizedText::Select(context_.language, L"[실패]", L"[MISS]"));
            const Color color = score >= 0.86f
                ? Color::BrightCyan
                : (score >= 0.62f ? Color::BrightYellow : Color::BrightRed);
            screen.Text(53 + index * 9, 27, text, color);
        }

        screen.CenterText(
            31,
            LocalizedText::Select(
                context_.language,
                L"A / D : 온도 조절     SPACE : 타격     ESC : 중단",
                L"A / D : TEMPER HEAT     SPACE : STRIKE     ESC : ABORT"),
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

        // 난수 생성과 판정 계산을 분리해 ForgeRules 자체는 결정론적으로 유지한다.
        const float randomRoll = context_.randomProvider.NextFloat(0.0f, 1.0f);
        context_.lastOutcome = context_.forgeRules.Resolve(
            context_.progress,
            session_->GetStrikeScores(),
            randomRoll);

        const ForgeOutcome& outcome = *context_.lastOutcome;
        const Color swordColor = context_.hudRenderer.GetSwordColor(outcome.newLevel);
        // 타격 불꽃을 결과 전용 파티클로 교체해 장면 전환의 시각적 경계를 만든다.
        context_.particles.Clear();
        context_.particles.SpawnResultBurst(
            outcome.succeeded,
            swordColor,
            90);
        return SceneTransition::To(SceneType::Result);
    }
}
