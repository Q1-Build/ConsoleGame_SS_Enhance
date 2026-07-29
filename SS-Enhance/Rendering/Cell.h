#pragma once

#include "Rendering/Color.h"

namespace ss
{
    /// 화면 한 칸에 출력할 문자와 전경색을 보관하는 값 타입이다.
    struct Cell
    {
        wchar_t glyph = L' ';
        Color color = Color::White;
    };
}
