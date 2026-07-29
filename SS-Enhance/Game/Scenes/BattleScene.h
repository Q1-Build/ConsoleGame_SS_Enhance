#pragma once

#include "Game/Domain/BattleSettlement.h"
#include "Game/Domain/BossBattle.h"
#include "Game/Scenes/BossRenderer.h"
#include "Game/Scenes/IScene.h"
#include "Rendering/Color.h"

#include <memory>
#include <string>

namespace ss
{
    class BattleSession;
    struct BossAttackResult;
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
        /// 전투 장면의 입력 가능 범위와 결과 표시 단계를 명시한다.
        enum class BattlePhase
        {
            Introduction,
            Combat,
            RewardSelection,
            Completed
        };

        /// 승리 후 좌우 입력으로 고를 수 있는 두 보상 성향이다.
        enum class RewardChoice
        {
            Gold,
            Memory
        };

        [[nodiscard]] SceneTransition UpdateIntroduction(float deltaSeconds);
        [[nodiscard]] SceneTransition UpdateCombat(float deltaSeconds);
        [[nodiscard]] SceneTransition UpdateRewardSelection(float deltaSeconds);
        [[nodiscard]] SceneTransition UpdateCompleted(float deltaSeconds);
        void ShowBossAttackResult(const BossAttackResult& result);
        void BeginBattleResult();
        void BeginRetreat();
        void ApplySelectedReward();

        [[nodiscard]] BattleReward GetSelectedReward() const noexcept;
        void DrawAttackWarning(IScreen& screen) const;
        void DrawRewardSelection(IScreen& screen) const;

        // 공유 상태는 비소유하고 현재 전투의 정의와 세션만 장면이 소유한다.
        SceneContext& context_;
        BossDefinition boss_;
        std::unique_ptr<BattleSession> session_;
        BossRenderer bossRenderer_;
        BattleSettlement settlement_;
        BattlePhase phase_ = BattlePhase::Introduction;
        RewardChoice rewardChoice_ = RewardChoice::Gold;
        bool wasRetreat_ = false;

        // 장면 단계, 격파 연출, 메시지 유지와 화면 흔들림 시간을 초 단위로 관리한다.
        float phaseTimeSeconds_ = 0.0f;
        float defeatTimeSeconds_ = 0.0f;
        float messageTimeSeconds_ = 0.0f;
        float shakeTimeSeconds_ = 0.0f;

        // 최근 입력 결과를 다음 판정까지 화면에 유지하기 위한 표현 상태다.
        int lastDamage_ = 0;
        std::wstring battleMessage_;
        Color messageColor_ = Color::White;
    };
}
