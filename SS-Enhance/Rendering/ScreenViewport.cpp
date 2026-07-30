#include "Rendering/ScreenViewport.h"

#include <algorithm>
#include <cassert>
#include <cmath>

namespace ss
{
    ScreenViewport::ScreenViewport(
        IScreen& target,
        int left,
        int top,
        int width,
        int height)
        : target_(target),
          left_(left),
          top_(top),
          width_(width),
          height_(height)
    {
        assert(left >= 0);
        assert(top >= 0);
        assert(width > 0);
        assert(height > 0);
    }

    void ScreenViewport::Clear(wchar_t glyph, Color color)
    {
        // 전체 프레임의 입력 오버레이를 지우지 않고 장면에 할당된 영역만 초기화한다.
        for (int y = 0; y < height_; ++y)
        {
            for (int x = 0; x < width_; ++x)
            {
                Put(x, y, glyph, color);
            }
        }
    }

    void ScreenViewport::Put(int x, int y, wchar_t glyph, Color color)
    {
        if (!IsInside(x, y))
        {
            return;
        }

        const std::wstring_view glyphView(&glyph, 1);
        if (MeasureText(glyphView) == 2 && x + 1 >= width_)
        {
            return;
        }
        target_.Put(left_ + x, top_ + y, glyph, color);
    }

    void ScreenViewport::Text(
        int x,
        int y,
        std::wstring_view text,
        Color color)
    {
        int cursorX = x;
        for (const wchar_t glyph : text)
        {
            // 전각 문자는 대상 화면과 같은 폭 계산을 사용해 뷰포트 경계와 정렬을 일치시킨다.
            const std::wstring_view glyphView(&glyph, 1);
            Put(cursorX, y, glyph, color);
            cursorX += MeasureText(glyphView);
        }
    }

    int ScreenViewport::MeasureText(std::wstring_view text) const noexcept
    {
        return target_.MeasureText(text);
    }

    void ScreenViewport::RightText(
        int right,
        int y,
        std::wstring_view text,
        Color color)
    {
        Text(right - MeasureText(text) + 1, y, text, color);
    }

    void ScreenViewport::CenterTextIn(
        int left,
        int right,
        int y,
        std::wstring_view text,
        Color color)
    {
        const int regionWidth = std::max(0, right - left + 1);
        const int textLeft =
            left + std::max(0, regionWidth - MeasureText(text)) / 2;
        Text(textLeft, y, text, color);
    }

    void ScreenViewport::CenterText(
        int y,
        std::wstring_view text,
        Color color)
    {
        // 전체 프레임이 아니라 기존 게임 영역을 기준으로 중앙을 계산한다.
        CenterTextIn(0, width_ - 1, y, text, color);
    }

    void ScreenViewport::Line(
        int x1,
        int y1,
        int x2,
        int y2,
        wchar_t glyph,
        Color color)
    {
        // 뷰포트 경계를 넘는 선도 Put에서 잘리도록 정수 좌표로 직접 전개한다.
        const int deltaX = std::abs(x2 - x1);
        const int stepX = x1 < x2 ? 1 : -1;
        const int deltaY = -std::abs(y2 - y1);
        const int stepY = y1 < y2 ? 1 : -1;
        int error = deltaX + deltaY;

        while (true)
        {
            Put(x1, y1, glyph, color);
            if (x1 == x2 && y1 == y2)
            {
                break;
            }

            const int doubledError = error * 2;
            if (doubledError >= deltaY)
            {
                error += deltaY;
                x1 += stepX;
            }
            if (doubledError <= deltaX)
            {
                error += deltaX;
                y1 += stepY;
            }
        }
    }

    void ScreenViewport::Box(
        int left,
        int top,
        int right,
        int bottom,
        Color color)
    {
        for (int x = left + 1; x < right; ++x)
        {
            Put(x, top, L'─', color);
            Put(x, bottom, L'─', color);
        }
        for (int y = top + 1; y < bottom; ++y)
        {
            Put(left, y, L'│', color);
            Put(right, y, L'│', color);
        }

        Put(left, top, L'┌', color);
        Put(right, top, L'┐', color);
        Put(left, bottom, L'└', color);
        Put(right, bottom, L'┘', color);
    }

    bool ScreenViewport::IsInside(int x, int y) const noexcept
    {
        return x >= 0 && x < width_ && y >= 0 && y < height_;
    }
}
