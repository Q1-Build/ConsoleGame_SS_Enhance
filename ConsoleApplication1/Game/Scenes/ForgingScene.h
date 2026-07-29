#pragma once

#include "Game/Scenes/IScene.h"

#include <memory>

namespace ss
{
    class ForgeSession;
    struct SceneContext;

    /// 실시간 온도 조절과 세 번의 타격을 진행하고 강화 판정을 요청하는 장면이다.
    class ForgingScene final : public IScene
    {
    public:
        explicit ForgingScene(SceneContext& context);
        ~ForgingScene() override;

        ForgingScene(const ForgingScene&) = delete;
        ForgingScene& operator=(const ForgingScene&) = delete;
        ForgingScene(ForgingScene&&) = delete;
        ForgingScene& operator=(ForgingScene&&) = delete;

        void OnEnter() override;
        [[nodiscard]] SceneTransition Update(float deltaSeconds) override;
        void Render(IScreen& screen) const override;
        void OnExit() override;

    private:
        [[nodiscard]] SceneTransition ResolveForge();

        // 공유 서비스는 비소유하며 제련 세션은 이 장면이 단독 소유한다.
        SceneContext& context_;
        std::unique_ptr<ForgeSession> session_;
    };
}
