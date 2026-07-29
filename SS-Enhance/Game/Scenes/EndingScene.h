#pragma once

#include "Game/Scenes/IScene.h"

namespace ss
{
    struct SceneContext;

    /// 최종 보스 처치 뒤 플레이 기록과 완성된 검을 보여주는 엔딩 장면이다.
    class EndingScene final : public IScene
    {
    public:
        explicit EndingScene(SceneContext& context);

        void OnEnter() override;
        [[nodiscard]] SceneTransition Update(float deltaSeconds) override;
        void Render(IScreen& screen) const override;
        void OnExit() override;

    private:
        SceneContext& context_;
        float sceneTimeSeconds_ = 0.0f;
    };
}
