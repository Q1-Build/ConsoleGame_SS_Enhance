#pragma once

namespace ss
{
    /// 강화 구간의 끝에서 상대하는 보스 종류다.
    enum class BossType
    {
        EmberWarden,
        StormSentinel,
        MemoryDevourer
    };

    /// 보스의 공격 주기와 타이밍 표시를 변화시키는 패턴 종류다.
    enum class BossPattern
    {
        HeavyStrike,
        TripleCombo,
        MemoryDistortion
    };

    /// 한 보스 전투에 필요한 순수 전투 수치와 보상을 묶는다.
    struct BossDefinition
    {
        // 보스 식별 값과 패턴은 화면 문구 및 세션별 특수 동작을 선택한다.
        BossType type = BossType::EmberWarden;
        BossPattern pattern = BossPattern::HeavyStrike;

        // 공격 수치는 체력, 기본 공격 간격과 사전 예고 시간을 초 단위로 표현한다.
        int maxHealth = 100;
        int attackDamage = 10;
        float attackIntervalSeconds = 2.0f;
        float warningSeconds = 0.8f;

        // 연속 공격이 없는 보스는 comboCount 1과 comboGapSeconds 0을 사용한다.
        int comboCount = 1;
        float comboGapSeconds = 0.0f;

        // 승리 시 한 번만 지급할 진행 보상이다.
        int rewardGold = 500;
        int rewardFragments = 1;
    };
}
