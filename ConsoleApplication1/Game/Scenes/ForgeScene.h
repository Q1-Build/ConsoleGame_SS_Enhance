#pragma once

#include "Game/Scenes/IScene.h"

namespace ss
{
    struct SceneContext;

    /// 검 상태와 강화 정보를 보여주고 제련 시작을 요청받는 대장간 장면이다.
    class ForgeScene final : public IScene
    {
    public:
        explicit ForgeScene(SceneContext& context);

        void OnEnter() override;
        [[nodiscard]] SceneTransition Update(float deltaSeconds) override;
        void Render(IScreen& screen) const override;
        void OnExit() override;

    private:
        // 공유 서비스의 수명은 GameApplication이 보장한다.
        SceneContext& context_;
    };
}
