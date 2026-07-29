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

        /// 전체 화면을 지정한 문자와 색상으로 초기화한다.
        virtual void Clear(wchar_t glyph = L' ', Color color = Color::White) = 0;

        /// 화면 범위 안의 한 셀을 변경하며 범위 밖 좌표는 무시한다.
        virtual void Put(int x, int y, wchar_t glyph, Color color = Color::White) = 0;

        /// 지정한 좌표부터 소유하지 않는 문자열 뷰를 그린다.
        virtual void Text(int x, int y, std::wstring_view text, Color color = Color::White) = 0;

        /// 전각 문자 폭을 반영해 지정한 오른쪽 좌표에 문자열 끝을 맞춘다.
        virtual void RightText(
            int right,
            int y,
            std::wstring_view text,
            Color color = Color::White) = 0;

        /// 전각 문자 폭을 반영해 지정한 좌우 영역 안에 문자열을 가운데 정렬한다.
        virtual void CenterTextIn(
            int left,
            int right,
            int y,
            std::wstring_view text,
            Color color = Color::White) = 0;

        virtual void CenterText(int y, std::wstring_view text, Color color = Color::White) = 0;
        virtual void Line(int x1, int y1, int x2, int y2, wchar_t glyph, Color color) = 0;
        virtual void Box(int left, int top, int right, int bottom, Color color) = 0;
    };
}
