#pragma once

#include "Rendering/Color.h"

#include <string_view>

namespace ss
{
    class IScreen;
    class PlayerProgress;

    /// 여러 장면이 공유하는 배경, 상태창, 검 형상을 일관되게 그린다.
    class GameHudRenderer final
    {
    public:
        void DrawBackdrop(IScreen& screen, float worldTimeSeconds) const;
        void DrawHeader(IScreen& screen, const PlayerProgress& progress) const;
        void DrawSword(
            IScreen& screen,
            int centerX,
            int topY,
            Color color,
            int level,
            float glow,
            float worldTimeSeconds) const;
        void DrawProgressBar(
            IScreen& screen,
            int x,
            int y,
            int width,
            float value,
            Color fillColor,
            std::wstring_view label) const;

        [[nodiscard]] Color GetSwordColor(int swordLevel) const noexcept;

    private:
        [[nodiscard]] static constexpr float Clamp01(float value) noexcept
        {
            return value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
        }
    };
}
