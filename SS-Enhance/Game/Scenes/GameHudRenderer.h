#pragma once

#include "Core/Language.h"
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
        /// 장면 공통 테두리와 하단 화로 애니메이션을 그린다.
        void DrawBackdrop(IScreen& screen, float worldTimeSeconds) const;

        /// 플레이어의 골드와 기억 조각을 공통 상단 영역에 표시한다.
        void DrawHeader(
            IScreen& screen,
            const PlayerProgress& progress,
            Language language) const;

        /// 강화 단계에 따라 길이와 오라가 달라지는 검 형상을 그린다.
        void DrawSword(
            IScreen& screen,
            int centerX,
            int topY,
            Color color,
            int level,
            float glow,
            float worldTimeSeconds) const;

        /// 0~1 범위의 값을 고정 폭 막대로 표현한다.
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
