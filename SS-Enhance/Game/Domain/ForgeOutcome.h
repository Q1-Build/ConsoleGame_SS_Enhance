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
        // 성공 종류와 최종 확률·제련도는 판정 시점에 확정되며 두 실수는 0~1 범위다.
        bool succeeded = false;
        bool wasCritical = false;
        float finalChance = 0.0f;
        float craftScore = 0.0f;

        // 판정 전후 강화 단계와 실패 시 실제로 적용된 보호·하락 결과다.
        int previousLevel = 0;
        int newLevel = 0;
        FailureConsequence failureConsequence = FailureConsequence::None;
    };
}
