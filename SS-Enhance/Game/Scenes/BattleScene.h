#pragma once

#include "Game/Domain/BossBattle.h"
#include "Game/Scenes/IScene.h"

#include <memory>

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
        void GrantVictoryReward();

        SceneContext& context_;
        BossDefinition boss_;
        std::unique_ptr<BattleSession> session_;
        float completedTimeSeconds_ = 0.0f;
        int lastDamage_ = 0;
        bool wasRewardGranted_ = false;
    };
}
