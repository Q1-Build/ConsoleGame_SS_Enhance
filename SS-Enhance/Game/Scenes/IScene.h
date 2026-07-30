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

        /// 장면 생성 직후 한 번 호출되며 첫 갱신 전에 유효한 장면 상태를 만든다.
        virtual void OnEnter() = 0;

        /// 경과 시간만큼 입력과 상태를 갱신하고 장면 전환 의도를 반환한다.
        /// @param deltaSeconds 이전 프레임 이후 경과한 초
        [[nodiscard]] virtual SceneTransition Update(float deltaSeconds) = 0;

        /// 현재 상태를 화면에 그리며 게임 상태는 변경하지 않는다.
        virtual void Render(IScreen& screen) const = 0;

        /// 장면 객체가 폐기되기 직전에 한 번 호출되어 임시 상태를 정리한다.
        virtual void OnExit() = 0;
    };
}
