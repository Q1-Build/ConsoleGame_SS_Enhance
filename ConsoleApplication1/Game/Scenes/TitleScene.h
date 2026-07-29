#pragma once

#include "Game/Scenes/IScene.h"

namespace ss
{
    struct SceneContext;

    /// 게임 제목과 시작 입력을 담당하는 장면이다.
    class TitleScene final : public IScene
    {
    public:
        explicit TitleScene(SceneContext& context);

        void OnEnter() override;
        [[nodiscard]] SceneTransition Update(float deltaSeconds) override;
        void Render(IScreen& screen) const override;
        void OnExit() override;

    private:
        // 공유 서비스의 수명은 GameApplication이 보장한다.
        SceneContext& context_;
    };
}
