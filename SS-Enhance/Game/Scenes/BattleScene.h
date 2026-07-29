#pragma once

#include "Game/Domain/BossBattle.h"
#include "Game/Scenes/IScene.h"
#include "Rendering/Color.h"

#include <memory>
#include <string>

namespace ss
{
    class BattleSession;
    struct SceneContext;

    /// 현재 강화 구간의 보스와 타이밍 전투를 진행하고 보상을 확정하는 장면이다.
    class BattleScene final : public IScene
    {
    public:
        explicit BattleScene(SceneContext& context);
        ~BattleScene() override;

        void OnEnter() override;
        [[nodiscard]] SceneTransition Update(float deltaSeconds) override;
        void Render(IScreen& screen) const override;
        void OnExit() override;

    private:
        /// 확정된 승패에 따라 보상 또는 단계 하락을 정확히 한 번 진행도에 반영한다.
        void ApplyBattleOutcome();

        // 공유 상태는 비소유하고 현재 전투의 정의와 세션만 장면이 소유한다.
        SceneContext& context_;
        BossDefinition boss_;
        std::unique_ptr<BattleSession> session_;

        // 결과 입력 지연, 메시지 유지와 화면 흔들림 시간을 초 단위로 관리한다.
        float completedTimeSeconds_ = 0.0f;
        float messageTimeSeconds_ = 0.0f;
        float shakeTimeSeconds_ = 0.0f;

        // 최근 입력 결과를 다음 판정까지 화면에 유지하기 위한 표현 상태다.
        int lastDamage_ = 0;
        std::wstring battleMessage_;
        Color messageColor_ = Color::White;

        // 여러 완료 프레임에서도 진행 보상이나 페널티가 중복 적용되지 않게 한다.
        bool wasBattleOutcomeApplied_ = false;
    };
}
