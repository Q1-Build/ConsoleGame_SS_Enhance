#pragma once

#include "Core/Language.h"
#include "Game/Scenes/IScene.h"

#include <string_view>

namespace ss
{
    struct ForgeOutcome;
    struct SceneContext;

    /// 직전 강화 결과와 단계 변화를 연출하고 대장간 복귀를 처리하는 장면이다.
    class ResultScene final : public IScene
    {
    public:
        explicit ResultScene(SceneContext& context);

        void OnEnter() override;
        [[nodiscard]] SceneTransition Update(float deltaSeconds) override;
        void Render(IScreen& screen) const override;
        void OnExit() override;

    private:
        [[nodiscard]] static std::wstring_view GetHeadline(
            const ForgeOutcome& outcome,
            Language language) noexcept;
        [[nodiscard]] static std::wstring_view GetDetail(
            const ForgeOutcome& outcome,
            Language language) noexcept;

        // 공유 서비스는 비소유하며 장면 경과 시간만 직접 관리한다.
        SceneContext& context_;
        float sceneTimeSeconds_ = 0.0f;
    };
}
