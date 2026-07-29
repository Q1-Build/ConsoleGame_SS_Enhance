#pragma once

#include "Rendering/Color.h"

#include <string_view>

namespace ss
{
    /// 장면이 구체적인 콘솔 구현을 몰라도 화면을 구성할 수 있게 하는 그리기 계약이다.
    class IScreen
    {
    public:
        virtual ~IScreen() = default;

        virtual void Clear(wchar_t glyph = L' ', Color color = Color::White) = 0;
        virtual void Put(int x, int y, wchar_t glyph, Color color = Color::White) = 0;
        virtual void Text(int x, int y, std::wstring_view text, Color color = Color::White) = 0;
        virtual void CenterText(int y, std::wstring_view text, Color color = Color::White) = 0;
        virtual void Line(int x1, int y1, int x2, int y2, wchar_t glyph, Color color) = 0;
        virtual void Box(int left, int top, int right, int bottom, Color color) = 0;
    };
}
