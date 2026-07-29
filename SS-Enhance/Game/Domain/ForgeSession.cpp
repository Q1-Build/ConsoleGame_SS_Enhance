#include "Game/Domain/ForgeSession.h"

#include <algorithm>
#include <cassert>
#include <cmath>

namespace ss
{
    ForgeSession::ForgeSession(int swordLevel, Difficulty difficulty)
        : swordLevel_(swordLevel)
    {
        assert(swordLevel >= 0);

        // 세션 중 바뀌지 않는 난이도 수치만 복사해 이후 프레임 갱신을 단순하게 유지한다.
        const DifficultyTuning tuning = GetDifficultyTuning(difficulty);
        naturalCoolingMultiplier_ = tuning.naturalCoolingMultiplier;
        timeLeft_ = tuning.forgeDurationSeconds;

        strikeScores_.reserve(3);
    }

    void ForgeSession::Update(
        float deltaSeconds,
        float worldTimeSeconds,
        bool isCooling,
        bool isStoking)
    {
        assert(deltaSeconds >= 0.0f);

        timeLeft_ -= deltaSeconds;
        strikeCooldown_ = std::max(0.0f, strikeCooldown_ - deltaSeconds);
        impactFlash_ = std::max(0.0f, impactFlash_ - deltaSeconds);

        // 강화 단계가 높을수록 리듬 왕복 속도를 조금씩 높여 체감 난도를 올린다.
        const float rhythmSpeed = 2.25f + static_cast<float>(swordLevel_) * 0.11f;
        marker_ = std::sin(worldTimeSeconds * rhythmSpeed) * 0.5f + 0.5f;

        // 높은 난이도에서는 자연 냉각만 빨라져 가열 입력과 타격 시점 관리가 더 중요해진다.
        const float naturalCooling =
            (5.5f + static_cast<float>(swordLevel_) * 0.15f) *
            naturalCoolingMultiplier_;
        heat_ -= deltaSeconds * naturalCooling;
        if (isCooling)
        {
            heat_ -= deltaSeconds * 31.0f;
        }
        if (isStoking)
        {
            heat_ += deltaSeconds * 39.0f;
        }
        heat_ = Clamp(heat_, 0.0f, 100.0f);
    }

    std::optional<float> ForgeSession::TryStrike()
    {
        if (strikeCooldown_ > 0.0f || IsComplete())
        {
            return std::nullopt;
        }

        const float markerAccuracy = 1.0f - std::abs(marker_ - 0.5f) * 2.0f;
        const float heatAccuracy = 1.0f - std::abs(heat_ - 68.0f) / 32.0f;

        // 리듬 조작 비중을 더 크게 두되 온도를 무시하면 최고 점수를 얻을 수 없게 한다.
        const float score = Clamp(
            markerAccuracy * 0.68f + heatAccuracy * 0.32f,
            0.0f,
            1.0f);

        strikeScores_.push_back(score);

        // 연속 키 입력을 막고 타격 직후 열이 빠지는 손맛을 상태 변화로 표현한다.
        strikeCooldown_ = 0.36f;
        impactFlash_ = 0.16f;
        heat_ = Clamp(heat_ - 12.0f, 0.0f, 100.0f);
        return score;
    }

    bool ForgeSession::IsComplete() const noexcept
    {
        return strikeScores_.size() >= 3 || timeLeft_ <= 0.0f;
    }

    float ForgeSession::GetHeat() const noexcept
    {
        return heat_;
    }

    float ForgeSession::GetMarker() const noexcept
    {
        return marker_;
    }

    float ForgeSession::GetTimeLeft() const noexcept
    {
        return timeLeft_;
    }

    float ForgeSession::GetStrikeCooldown() const noexcept
    {
        return strikeCooldown_;
    }

    float ForgeSession::GetImpactFlash() const noexcept
    {
        return impactFlash_;
    }

    const std::vector<float>& ForgeSession::GetStrikeScores() const noexcept
    {
        return strikeScores_;
    }
}
