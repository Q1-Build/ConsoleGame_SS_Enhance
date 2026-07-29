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

    /// 한 보스 전투에 필요한 순수 전투 수치와 보상을 묶는다.
    struct BossDefinition
    {
        BossType type = BossType::EmberWarden;
        int maxHealth = 100;
        int attackDamage = 10;
        float attackIntervalSeconds = 2.0f;
        int rewardGold = 500;
        int rewardFragments = 1;
    };
}
