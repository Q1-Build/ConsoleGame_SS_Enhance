#pragma once

#include "Game/Scenes/IScene.h"

namespace ss
{
    struct SceneContext;

    /// 게임 제목과 시작 입력을 담당하는 장면이다.
    class TitleScene final : public IScene
    {
    public:
        /// 애플리케이션이 소유한 공유 상태와 서비스를 비소유 참조로 연결한다.
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
