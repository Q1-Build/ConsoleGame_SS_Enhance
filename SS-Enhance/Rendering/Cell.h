#pragma once

#include "Rendering/Color.h"

namespace ss
{
    /// 화면 한 칸에 출력할 문자와 전경색을 보관하는 값 타입이다.
    struct Cell
    {
        // 셀에 표시할 문자와 ANSI 전경색이다.
        wchar_t glyph = L' ';
        Color color = Color::White;

        // 전각 문자가 차지한 두 번째 화면 칸은 출력 문자열에 다시 쓰지 않는다.
        bool isWideContinuation = false;
    };
}
