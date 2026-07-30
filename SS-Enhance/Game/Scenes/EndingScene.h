#pragma once

#include "Game/Scenes/IScene.h"

namespace ss
{
    struct SceneContext;

    /// 최종 보스 처치 뒤 플레이 기록과 완성된 검을 보여주는 엔딩 장면이다.
    class EndingScene final : public IScene
    {
    public:
        /// 애플리케이션이 소유한 최종 진행 기록과 공유 서비스를 비소유 참조로 연결한다.
        explicit EndingScene(SceneContext& context);

        void OnEnter() override;
        [[nodiscard]] SceneTransition Update(float deltaSeconds) override;
        void Render(IScreen& screen) const override;
        void OnExit() override;

    private:
        // 공유 서비스는 비소유하며 엔딩 입력 지연을 위한 초 단위 경과 시간만 직접 관리한다.
        SceneContext& context_;
        float sceneTimeSeconds_ = 0.0f;
    };
}
