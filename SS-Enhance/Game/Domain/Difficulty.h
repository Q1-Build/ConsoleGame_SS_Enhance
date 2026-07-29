#pragma once

namespace ss
{
    /// 강화 비용, 제련 조건과 실패 페널티 시작 단계에 적용할 게임 난이도다.
    enum class Difficulty
    {
        /// 비용과 냉각 부담을 낮추고 +10 전까지 실패 페널티를 유예한다.
        Easy,

        /// 기준 비용과 제련 조건을 사용하며 +9부터 실패 페널티를 적용한다.
        Normal,

        /// 비용과 냉각 부담을 높이고 기존 기준인 +4부터 실패 페널티를 적용한다.
        Hard
    };

    /// 한 난이도에 묶여 함께 조정되는 강화 비용과 제련 조건이다.
    struct DifficultyTuning
    {
        // 비용, 냉각, 제한 시간과 실패 페널티 시작 단계를 함께 관리한다.
        int forgeCostPercent = 100;
        float naturalCoolingMultiplier = 1.0f;
        float forgeDurationSeconds = 12.0f;
        int failurePenaltyStartLevel = 9;
    };

    /// 난이도별 밸런스 값을 한곳에서 반환해 비용과 제련 규칙의 수치 중복을 막는다.
    [[nodiscard]] constexpr DifficultyTuning GetDifficultyTuning(
        Difficulty difficulty) noexcept
    {
        switch (difficulty)
        {
        case Difficulty::Easy:
            return {90, 0.85f, 14.0f, 10};
        case Difficulty::Normal:
            return {100, 1.0f, 12.0f, 9};
        case Difficulty::Hard:
            return {115, 1.25f, 10.0f, 4};
        }
        return {};
    }
}
