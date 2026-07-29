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

        [[nodiscard]] bool IsComplete() const noexcept;
        [[nodiscard]] float GetHeat() const noexcept;
        [[nodiscard]] float GetMarker() const noexcept;
        [[nodiscard]] float GetTimeLeft() const noexcept;
        [[nodiscard]] float GetStrikeCooldown() const noexcept;
        [[nodiscard]] float GetImpactFlash() const noexcept;
        [[nodiscard]] const std::vector<float>& GetStrikeScores() const noexcept;

    private:
        [[nodiscard]] static constexpr float Clamp(
            float value,
            float minValue,
            float maxValue) noexcept
        {
            return value < minValue ? minValue : (value > maxValue ? maxValue : value);
        }

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
