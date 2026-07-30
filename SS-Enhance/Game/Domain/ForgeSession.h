#pragma once

#include "Game/Domain/Difficulty.h"

#include <optional>
#include <vector>

namespace ss
{
    /// 한 번의 실시간 제련에서 온도, 리듬, 제한 시간, 타격 점수를 관리한다.
    class ForgeSession final
    {
    public:
        /// 진입 시점의 검 단계와 난이도로 한 번의 제련 조건을 고정한다.
        ForgeSession(int swordLevel, Difficulty difficulty);

        /// 경과 시간, 전체 게임 시간과 온도 조작 입력으로 실시간 제련 상태를 갱신한다.
        /// 리듬 표식은 전체 게임 시간을 사용하지만 점수와 제한 시간은 세션 안에서 관리한다.
        void Update(
            float deltaSeconds,
            float worldTimeSeconds,
            bool isCooling,
            bool isStoking);

        /// 현재 타격을 시도하고 성공적으로 입력된 타격 점수를 반환한다.
        [[nodiscard]] std::optional<float> TryStrike();

        /// 세 번 타격했거나 제한 시간이 끝나 최종 판정을 요청해야 하는지 반환한다.
        [[nodiscard]] bool IsComplete() const noexcept;

        /// UI 표현에 필요한 현재 온도, 리듬 위치와 초 단위 타이머를 조회한다.
        [[nodiscard]] float GetHeat() const noexcept;
        [[nodiscard]] float GetMarker() const noexcept;
        [[nodiscard]] float GetTimeLeft() const noexcept;
        [[nodiscard]] float GetStrikeCooldown() const noexcept;
        [[nodiscard]] float GetImpactFlash() const noexcept;

        /// 입력 순서대로 저장된 0~1 타격 점수의 읽기 전용 참조를 반환한다.
        /// 반환 참조는 세션이 유지되고 다음 타격으로 벡터가 변경되기 전까지 유효하다.
        [[nodiscard]] const std::vector<float>& GetStrikeScores() const noexcept;

        /// 높은 온도 정확도를 얻는 64~72도 최적 공명 구간인지 반환한다.
        [[nodiscard]] static constexpr bool IsOptimalHeat(float heat) noexcept
        {
            return heat >= kOptimalHeatMinimum && heat <= kOptimalHeatMaximum;
        }

        /// 타격 점수를 안정적으로 얻는 58~78도 공명 구간인지 반환한다.
        [[nodiscard]] static constexpr bool IsResonantHeat(float heat) noexcept
        {
            return heat >= kResonantHeatMinimum && heat <= kResonantHeatMaximum;
        }

        /// 최적 공명 구간의 최저 온도를 반환한다.
        [[nodiscard]] static constexpr float GetOptimalHeatMinimum() noexcept
        {
            return kOptimalHeatMinimum;
        }

        /// 최적 공명 구간의 최고 온도를 반환한다.
        [[nodiscard]] static constexpr float GetOptimalHeatMaximum() noexcept
        {
            return kOptimalHeatMaximum;
        }

    private:
        [[nodiscard]] static constexpr float Clamp(
            float value,
            float minValue,
            float maxValue) noexcept
        {
            return value < minValue ? minValue : (value > maxValue ? maxValue : value);
        }

        // 68도를 중심으로 점수를 계산하며 UI 안내 범위도 같은 기준에서 파생한다.
        static constexpr float kIdealHeat = 68.0f;
        static constexpr float kHeatAccuracySpan = 32.0f;
        static constexpr float kOptimalHeatMinimum = 64.0f;
        static constexpr float kOptimalHeatMaximum = 72.0f;
        static constexpr float kResonantHeatMinimum = 58.0f;
        static constexpr float kResonantHeatMaximum = 78.0f;

        // 진입 시 고정된 제련 조건과 프레임마다 변하는 실시간 상태다.
        int swordLevel_ = 0;
        float naturalCoolingMultiplier_ = 1.0f;
        float heat_ = 50.0f;
        float marker_ = 0.0f;
        float timeLeft_ = 0.0f;
        float strikeCooldown_ = 0.0f;
        float impactFlash_ = 0.0f;

        // 최대 세 번의 타격 정확도를 0~1 범위로 저장한다.
        std::vector<float> strikeScores_;
    };
}
