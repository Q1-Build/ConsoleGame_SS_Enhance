#pragma once

#include "Core/Language.h"
#include "Game/Domain/Difficulty.h"
#include "Game/Domain/ForgeOutcome.h"

#include <optional>
#include <string>

namespace ss
{
    class ForgeRules;
    class GameHudRenderer;
    class IInput;
    class IRandomProvider;
    class ParticleSystem;
    class PlayerProgress;

    /// 장면들이 공유하는 게임 상태와 서비스의 비소유 참조를 묶는다.
    struct SceneContext
    {
        SceneContext(
            IInput& inputReference,
            IRandomProvider& randomReference,
            PlayerProgress& progressReference,
            ForgeRules& rulesReference,
            ParticleSystem& particlesReference,
            GameHudRenderer& hudRendererReference)
            : input(inputReference),
              randomProvider(randomReference),
              progress(progressReference),
              forgeRules(rulesReference),
              particles(particlesReference),
              hudRenderer(hudRendererReference)
        {
        }

        // 참조 대상은 GameApplication이 소유하며 모든 장면보다 오래 살아야 한다.
        IInput& input;
        IRandomProvider& randomProvider;
        PlayerProgress& progress;
        ForgeRules& forgeRules;
        ParticleSystem& particles;
        GameHudRenderer& hudRenderer;

        // 전체 장면이 공유하는 시간과 직전 강화 결과다.
        float worldTimeSeconds = 0.0f;
        std::optional<ForgeOutcome> lastOutcome;
        std::wstring notice;

        // 첫 실행은 한국어와 보통 난이도이며 설정 장면에서 본게임 진입 전에 변경한다.
        Language language = Language::Korean;

        Difficulty difficulty = Difficulty::Normal;
    };
}
