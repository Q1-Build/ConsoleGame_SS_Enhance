#pragma once

#include "Game/Domain/ForgeRules.h"
#include "Game/Domain/PlayerProgress.h"
#include "Game/Effects/ParticleSystem.h"
#include "Game/Scenes/GameHudRenderer.h"
#include "Game/Scenes/InputOverlay.h"
#include "Game/Scenes/SceneContext.h"
#include "Rendering/ScreenBuffer.h"
#include "Rendering/ScreenViewport.h"

#include <memory>

namespace ss
{
    class IFramePresenter;
    class IInput;
    class IRandomProvider;
    class IScene;
    enum class SceneType;

    /// 프레임 루프와 장면 수명 주기를 관리하는 애플리케이션 진입점이다.
    class GameApplication final
    {
    public:
        GameApplication(
            IInput& input,
            IFramePresenter& presenter,
            IRandomProvider& randomProvider);
        ~GameApplication();

        GameApplication(const GameApplication&) = delete;
        GameApplication& operator=(const GameApplication&) = delete;
        GameApplication(GameApplication&&) = delete;
        GameApplication& operator=(GameApplication&&) = delete;

        /// 종료 장면이 요청될 때까지 게임 루프를 실행하고 프로세스 종료 코드를 반환한다.
        [[nodiscard]] int Run();

    private:
        void ChangeScene(SceneType sceneType);
        [[nodiscard]] std::unique_ptr<IScene> CreateScene(SceneType sceneType);

        // 플랫폼 구현은 main이 소유하며 애플리케이션보다 오래 사는 비소유 참조다.
        IInput& input_;
        IFramePresenter& presenter_;
        IRandomProvider& randomProvider_;

        // 게임 전체에서 한 번 생성되어 장면들이 공유하는 상태와 서비스다.
        ScreenBuffer screen_;
        ScreenViewport gameScreen_;
        PlayerProgress progress_;
        ForgeRules forgeRules_;
        ParticleSystem particles_;
        GameHudRenderer hudRenderer_;
        InputOverlay inputOverlay_;
        SceneContext sceneContext_;

        // 현재 장면은 애플리케이션이 단독 소유하며 전환 시 즉시 교체한다.
        std::unique_ptr<IScene> currentScene_;
        bool isRunning_ = false;
    };
}
