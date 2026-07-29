#pragma once

#include "Rendering/Cell.h"
#include "Rendering/IScreen.h"

#include <string>
#include <vector>

namespace ss
{
    /// 장면이 그린 문자를 메모리에 보관하고 하나의 ANSI 프레임으로 조립한다.
    class ScreenBuffer final : public IScreen
    {
    public:
        ScreenBuffer();

        void Clear(wchar_t glyph = L' ', Color color = Color::White) override;
        void Put(int x, int y, wchar_t glyph, Color color = Color::White) override;
        void Text(int x, int y, std::wstring_view text, Color color = Color::White) override;
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
        void CenterText(int y, std::wstring_view text, Color color = Color::White) override;
        void Line(int x1, int y1, int x2, int y2, wchar_t glyph, Color color) override;
        void Box(int left, int top, int right, int bottom, Color color) override;

        /// 현재 버퍼를 커서 이동과 색상 코드를 포함한 출력 문자열로 만든다.
        [[nodiscard]] std::wstring BuildAnsiFrame() const;

    private:
        [[nodiscard]] bool IsInside(int x, int y) const noexcept;
        [[nodiscard]] static int GetDisplayWidth(wchar_t glyph) noexcept;
        [[nodiscard]] static int GetDisplayWidth(std::wstring_view text) noexcept;
        void ClearOccupiedCell(int x, int y);

        // 화면 좌표를 행 우선 순서로 저장하는 고정 크기 셀 버퍼다.
        std::vector<Cell> cells_;
    };
}
