#pragma once

#include "Game/Scenes/SceneTransition.h"

namespace ss
{
    class IScreen;

    /// 모든 게임 장면이 따르는 진입, 갱신, 그리기, 종료 수명 주기 계약이다.
    class IScene
    {
    public:
        virtual ~IScene() = default;

        virtual void OnEnter() = 0;
        [[nodiscard]] virtual SceneTransition Update(float deltaSeconds) = 0;
        virtual void Render(IScreen& screen) const = 0;
        virtual void OnExit() = 0;
    };
}
