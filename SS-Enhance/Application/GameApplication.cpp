#include "Application/GameApplication.h"

#include "Core/GameConstants.h"
#include "Core/IRandomProvider.h"
#include "Game/Scenes/BattleScene.h"
#include "Game/Scenes/EndingScene.h"
#include "Game/Scenes/ForgeScene.h"
#include "Game/Scenes/ForgingScene.h"
#include "Game/Scenes/IScene.h"
#include "Game/Scenes/ResultScene.h"
#include "Game/Scenes/SettingsScene.h"
#include "Game/Scenes/TitleScene.h"
#include "Platform/IAudio.h"
#include "Platform/IInput.h"
#include "Rendering/IFramePresenter.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <thread>

namespace ss
{
    GameApplication::GameApplication(
        IInput& input,
        IFramePresenter& presenter,
        IRandomProvider& randomProvider,
        IAudio& audio)
        : input_(input),
          presenter_(presenter),
          gameScreen_(
              screen_,
              0,
              0,
              kGameViewportWidth,
              kGameViewportHeight),
          particles_(randomProvider),
          sceneContext_(
              input_,
              randomProvider,
              progress_,
              forgeRules_,
              particles_,
              hudRenderer_,
              audio)
    {
    }

    GameApplication::~GameApplication() = default;

    int GameApplication::Run()
    {
        using Clock = std::chrono::steady_clock;

        isRunning_ = true;
        ChangeScene(SceneType::Title);
        auto previousTime = Clock::now();

        while (isRunning_)
        {
            const auto now = Clock::now();
            float deltaSeconds = std::chrono::duration<float>(now - previousTime).count();
            previousTime = now;

            // 디버거 중단이나 창 이동 뒤의 큰 시간 값이 게임 상태를 건너뛰지 않게 제한한다.
            deltaSeconds = std::min(deltaSeconds, 0.05f);

            // 발표 설명을 입력하는 동안 같은 키가 게임 조작으로 전달되지 않게 장면 갱신을 멈춘다.
            input_.Update();
            const bool isChatInputConsumed =
                presentationChatOverlay_.Update(input_, screen_);
            inputOverlay_.Update(deltaSeconds, input_);
            if (!isChatInputConsumed)
            {
                sceneContext_.worldTimeSeconds += deltaSeconds;
                particles_.Update(deltaSeconds);

                const SceneTransition transition =
                    currentScene_->Update(deltaSeconds);
                if (transition.isRequested)
                {
                    ChangeScene(transition.target);
                }
            }
            if (!isRunning_)
            {
                break;
            }

            // 장면은 기존 게임 뷰포트에, 발표용 입력 UI는 전체 프레임에 서로 겹치지 않게 그린다.
            screen_.Clear();
            currentScene_->Render(gameScreen_);
            particles_.Draw(gameScreen_);
            inputOverlay_.Draw(screen_, sceneContext_.language);
            presentationChatOverlay_.Draw(screen_, sceneContext_.language);

            // 여러 번 출력하면 깜빡임이 생기므로 완성된 프레임을 한 번에 전달한다.
            const std::wstring frame = screen_.BuildAnsiFrame();
            presenter_.Present(frame);

            std::this_thread::sleep_for(
                std::chrono::milliseconds(kTargetFrameMilliseconds));
        }

        return 0;
    }

    void GameApplication::ChangeScene(SceneType sceneType)
    {
        // 종료 콜백을 먼저 호출해 장면이 소유한 임시 상태를 정리한 뒤 객체를 폐기한다.
        if (currentScene_ != nullptr)
        {
            currentScene_->OnExit();
            currentScene_.reset();
        }

        if (sceneType == SceneType::Exit)
        {
            isRunning_ = false;
            return;
        }

        currentScene_ = CreateScene(sceneType);
        assert(currentScene_ != nullptr);

        // 새 장면은 생성 직후 한 번만 초기화하며 첫 Update 전에 유효한 상태를 만든다.
        currentScene_->OnEnter();
    }

    std::unique_ptr<IScene> GameApplication::CreateScene(SceneType sceneType)
    {
        // 구체 장면 생성은 Composition Root에 가까운 애플리케이션 경계 한곳에 모은다.
        switch (sceneType)
        {
        case SceneType::Title:
            return std::make_unique<TitleScene>(sceneContext_);
        case SceneType::Settings:
            return std::make_unique<SettingsScene>(sceneContext_);
        case SceneType::Forge:
            return std::make_unique<ForgeScene>(sceneContext_);
        case SceneType::Forging:
            return std::make_unique<ForgingScene>(sceneContext_);
        case SceneType::Result:
            return std::make_unique<ResultScene>(sceneContext_);
        case SceneType::Battle:
            return std::make_unique<BattleScene>(sceneContext_);
        case SceneType::Ending:
            return std::make_unique<EndingScene>(sceneContext_);
        case SceneType::Exit:
            break;
        }

        assert(false && "생성할 수 없는 장면 종류다.");
        return nullptr;
    }
}
