#pragma once

#include <vector>

namespace ss
{
    /// 강화 구간의 끝에서 상대하는 보스 종류다.
    enum class BossType
    {
        EmberWarden,
        StormSentinel,
        MemoryDevourer
    };

    /// 공격 예고의 성격을 구분해 UI가 진짜 공격과 기억의 잔상을 표현하게 한다.
    enum class AttackTelegraph
    {
        Honest,
        Distorted,
        Feint
    };

    /// 보스 공격 시퀀스의 한 단계를 구성하는 순수 전투 데이터다.
    struct BossAttackStep
    {
        // 단계 시작부터 판정까지의 시간과 그중 방어 예고로 보여 줄 시간을 초 단위로 저장한다.
        float delaySeconds = 2.0f;
        float warningSeconds = 0.8f;
        int damage = 10;
        AttackTelegraph telegraph = AttackTelegraph::Honest;
    };

    /// 승리 후 선택할 수 있는 골드와 기억 조각 보상 묶음이다.
    struct BattleReward
    {
        // 두 값은 진행도에 더할 음수 없는 정수 보상이다.
        int gold = 0;
        int fragments = 0;
    };

    /// 한 보스 전투에 필요한 전투 수치, 공격 시퀀스와 선택 보상을 묶는다.
    struct BossDefinition
    {
        // 보스 식별 값은 화면 이름과 전용 형상을 선택한다.
        BossType type = BossType::EmberWarden;

        // 체력과 순서대로 반복할 공격 단계를 전투 세션에 전달한다.
        int maxHealth = 100;
        std::vector<BossAttackStep> attackSequence;

        // 두 선택지는 성격은 다르지만 비슷한 기대 가치를 갖도록 보스별로 조정한다.
        BattleReward goldReward;
        BattleReward memoryReward;
    };
}
