#include "Application/GameApplication.h"

#include "Core/GameConstants.h"
#include "Core/IRandomProvider.h"
#include "Game/Scenes/ForgeScene.h"
#include "Game/Scenes/ForgingScene.h"
#include "Game/Scenes/IScene.h"
#include "Game/Scenes/ResultScene.h"
#include "Game/Scenes/TitleScene.h"
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
        IRandomProvider& randomProvider)
        : input_(input),
          presenter_(presenter),
          randomProvider_(randomProvider),
          particles_(randomProvider_),
          sceneContext_(
              input_,
              randomProvider_,
              progress_,
              forgeRules_,
              particles_,
              hudRenderer_)
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
            deltaSeconds = std::min(deltaSeconds, 0.05f);

            input_.Update();
            sceneContext_.worldTimeSeconds += deltaSeconds;
            particles_.Update(deltaSeconds);

            const SceneTransition transition = currentScene_->Update(deltaSeconds);
            if (transition.isRequested)
            {
                ChangeScene(transition.target);
            }
            if (!isRunning_)
            {
                break;
            }

            currentScene_->Render(screen_);
            particles_.Draw(screen_);
            const std::wstring frame = screen_.BuildAnsiFrame();
            presenter_.Present(frame);

            std::this_thread::sleep_for(
                std::chrono::milliseconds(kTargetFrameMilliseconds));
        }

        return 0;
    }

    void GameApplication::ChangeScene(SceneType sceneType)
    {
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
        currentScene_->OnEnter();
    }

    std::unique_ptr<IScene> GameApplication::CreateScene(SceneType sceneType)
    {
        // 구체 장면 생성은 Composition Root에 가까운 애플리케이션 경계 한곳에 모은다.
        switch (sceneType)
        {
        case SceneType::Title:
            return std::make_unique<TitleScene>(sceneContext_);
        case SceneType::Forge:
            return std::make_unique<ForgeScene>(sceneContext_);
        case SceneType::Forging:
            return std::make_unique<ForgingScene>(sceneContext_);
        case SceneType::Result:
            return std::make_unique<ResultScene>(sceneContext_);
        case SceneType::Exit:
            break;
        }

        assert(false && "생성할 수 없는 장면 종류다.");
        return nullptr;
    }
}
