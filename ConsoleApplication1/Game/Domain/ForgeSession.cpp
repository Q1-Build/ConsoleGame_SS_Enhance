#include "Game/Domain/ForgeSession.h"

#include <algorithm>
#include <cassert>
#include <cmath>

namespace ss
{
    ForgeSession::ForgeSession(int swordLevel)
        : swordLevel_(swordLevel)
    {
        assert(swordLevel >= 0);
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

        const float rhythmSpeed = 2.25f + static_cast<float>(swordLevel_) * 0.11f;
        marker_ = std::sin(worldTimeSeconds * rhythmSpeed) * 0.5f + 0.5f;

        heat_ -= deltaSeconds * (5.5f + static_cast<float>(swordLevel_) * 0.15f);
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
        const float score = Clamp(
            markerAccuracy * 0.68f + heatAccuracy * 0.32f,
            0.0f,
            1.0f);

        strikeScores_.push_back(score);
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
