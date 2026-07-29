#pragma once

namespace ss
{
    /// 강화 실패 시 적용된 보호 또는 하락 결과를 구분한다.
    enum class FailureConsequence
    {
        None,
        LevelMaintained,
        FragmentConsumed,
        LevelLost
    };

    /// 한 번의 강화 판정과 진행도 변화를 화면 계층에 전달하는 값 타입이다.
    struct ForgeOutcome
    {
        bool succeeded = false;
        bool wasCritical = false;
        float finalChance = 0.0f;
        float craftScore = 0.0f;
        int previousLevel = 0;
        int newLevel = 0;
        FailureConsequence failureConsequence = FailureConsequence::None;
    };
}
