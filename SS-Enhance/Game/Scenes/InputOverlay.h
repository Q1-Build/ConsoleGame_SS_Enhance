#pragma once

#include "Core/Language.h"
#include "Platform/InputKey.h"
#include "Rendering/Color.h"

#include <array>
#include <cstddef>
#include <string_view>

namespace ss
{
    class IInput;
    class IScreen;

    /// 발표용 조작 안내와 현재 입력을 보여 주는 가상 키보드 오버레이다.
    /// 게임 장면의 좌표와 상태는 변경하지 않고 입력 표현만 담당한다.
    class InputOverlay final
    {
    public:
        /// 현재 입력을 읽고 짧은 키 입력도 보이도록 강조 시간을 갱신한다.
        void Update(float deltaSeconds, const IInput& input);

        /// 오른쪽 조작 안내와 아래 가상 키보드를 전체 화면에 그린다.
        void Draw(IScreen& screen, Language language) const;

    private:
        /// 새 입력 점멸과 계속 누르는 상태를 서로 다른 색으로 표현한다.
        enum class KeyVisualState
        {
            Idle,
            Held,
            Pressed
        };

        void DrawControlGuide(IScreen& screen, Language language) const;
        void DrawVirtualKeyboard(IScreen& screen, Language language) const;
        void DrawKey(
            IScreen& screen,
            int left,
            int top,
            int right,
            int bottom,
            std::wstring_view label,
            InputKey key) const;
        void DrawGuideRow(
            IScreen& screen,
            int y,
            std::wstring_view keys,
            std::wstring_view description,
            InputKey primaryKey,
            InputKey secondaryKey) const;

        [[nodiscard]] KeyVisualState GetVisualState(
            InputKey key) const noexcept;
        [[nodiscard]] KeyVisualState GetVisualState(
            InputKey primaryKey,
            InputKey secondaryKey) const noexcept;
        [[nodiscard]] static Color GetHighlightColor(
            KeyVisualState state) noexcept;

        // 노란 점멸은 새 입력만 짧게 남기고, 계속 누름은 별도 청록 상태로 추적한다.
        std::array<
            float,
            static_cast<std::size_t>(InputKey::Count)> pressFlashSeconds_{};
        std::array<
            bool,
            static_cast<std::size_t>(InputKey::Count)> isHeld_{};
    };
}
