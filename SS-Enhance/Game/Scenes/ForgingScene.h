#pragma once

#include "Game/Scenes/IScene.h"

#include <memory>

namespace ss
{
    class ForgeSession;
    struct SceneContext;
    enum class Color;

    /// 실시간 온도 조절과 세 번의 타격을 진행하고 강화 판정을 요청하는 장면이다.
    class ForgingScene final : public IScene
    {
    public:
        /// 애플리케이션이 소유한 공유 상태를 연결하고 제련 세션은 진입 시 별도로 생성한다.
        explicit ForgingScene(SceneContext& context);

        /// 헤더에 전방 선언한 제련 세션을 완전한 타입이 보이는 구현 파일에서 폐기한다.
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
        void DrawHeatGauge(IScreen& screen, float heat, Color fillColor) const;
        [[nodiscard]] SceneTransition ResolveForge();

        // 공유 서비스는 비소유하며 제련 세션은 이 장면이 단독 소유한다.
        SceneContext& context_;
        std::unique_ptr<ForgeSession> session_;
    };
}
