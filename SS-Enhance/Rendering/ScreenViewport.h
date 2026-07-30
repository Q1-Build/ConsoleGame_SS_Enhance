#pragma once

#include "Rendering/IScreen.h"

namespace ss
{
    /// 전체 화면의 지정된 사각 영역을 독립된 화면처럼 제공한다.
    /// 장면이 공통 보조 UI의 크기와 위치를 알지 못하도록 좌표와 중앙 정렬을 격리한다.
    class ScreenViewport final : public IScreen
    {
    public:
        /// 대상 화면의 지정 영역을 비소유 참조로 연결한다.
        ScreenViewport(
            IScreen& target,
            int left,
            int top,
            int width,
            int height);

        void Clear(wchar_t glyph = L' ', Color color = Color::White) override;
        void Put(
            int x,
            int y,
            wchar_t glyph,
            Color color = Color::White) override;
        void Text(
            int x,
            int y,
            std::wstring_view text,
            Color color = Color::White) override;
        [[nodiscard]] int MeasureText(
            std::wstring_view text) const noexcept override;
        void RightText(
            int right,
            int y,
            std::wstring_view text,
            Color color = Color::White) override;
        void CenterTextIn(
            int left,
            int right,
            int y,
            std::wstring_view text,
            Color color = Color::White) override;
        void CenterText(
            int y,
            std::wstring_view text,
            Color color = Color::White) override;
        void Line(
            int x1,
            int y1,
            int x2,
            int y2,
            wchar_t glyph,
            Color color) override;
        void Box(
            int left,
            int top,
            int right,
            int bottom,
            Color color) override;

    private:
        [[nodiscard]] bool IsInside(int x, int y) const noexcept;

        // 대상 화면은 GameApplication이 소유하며 이 뷰포트보다 오래 살아야 한다.
        IScreen& target_;
        int left_ = 0;
        int top_ = 0;
        int width_ = 0;
        int height_ = 0;
    };
}
