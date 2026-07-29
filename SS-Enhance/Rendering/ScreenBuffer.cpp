#include "Rendering/ScreenBuffer.h"

#include "Core/GameConstants.h"

#include <algorithm>
#include <cmath>

namespace ss
{
    ScreenBuffer::ScreenBuffer()
        : cells_(kScreenWidth * kScreenHeight)
    {
    }

    void ScreenBuffer::Clear(wchar_t glyph, Color color)
    {
        std::fill(cells_.begin(), cells_.end(), Cell{glyph, color, false});
    }

    void ScreenBuffer::Put(int x, int y, wchar_t glyph, Color color)
    {
        // 파티클처럼 화면 밖으로 이동할 수 있는 요소 때문에 경계 밖 쓰기는 안전하게 무시한다.
        if (!IsInside(x, y))
        {
            return;
        }

        const int displayWidth = GetDisplayWidth(glyph);
        if (displayWidth == 2 && !IsInside(x + 1, y))
        {
            return;
        }

        // 기존 전각 문자의 어느 칸을 덮어써도 소유 문자와 연속 칸을 함께 비운다.
        ClearOccupiedCell(x, y);
        if (displayWidth == 2)
        {
            ClearOccupiedCell(x + 1, y);
        }

        cells_[y * kScreenWidth + x] = {glyph, color, false};
        if (displayWidth == 2)
        {
            cells_[y * kScreenWidth + x + 1] = {L' ', color, true};
        }
    }

    void ScreenBuffer::Text(int x, int y, std::wstring_view text, Color color)
    {
        int cursorX = x;
        for (const wchar_t glyph : text)
        {
            Put(cursorX, y, glyph, color);
            cursorX += GetDisplayWidth(glyph);
        }
    }

    int ScreenBuffer::MeasureText(std::wstring_view text) const noexcept
    {
        return GetDisplayWidth(text);
    }

    void ScreenBuffer::CenterText(int y, std::wstring_view text, Color color)
    {
        const int left = (kScreenWidth - GetDisplayWidth(text)) / 2;
        Text(left, y, text, color);
    }

    void ScreenBuffer::RightText(
        int right,
        int y,
        std::wstring_view text,
        Color color)
    {
        const int left = right - GetDisplayWidth(text) + 1;
        Text(left, y, text, color);
    }

    void ScreenBuffer::CenterTextIn(
        int left,
        int right,
        int y,
        std::wstring_view text,
        Color color)
    {
        const int regionWidth = std::max(0, right - left + 1);
        const int textWidth = GetDisplayWidth(text);
        const int textLeft = left + std::max(0, regionWidth - textWidth) / 2;
        Text(textLeft, y, text, color);
    }

    void ScreenBuffer::Line(int x1, int y1, int x2, int y2, wchar_t glyph, Color color)
    {
        // 모든 기울기의 선을 정수 좌표로 그리기 위해 Bresenham 방식으로 셀을 선택한다.
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

    void ScreenBuffer::Box(int left, int top, int right, int bottom, Color color)
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

    std::wstring ScreenBuffer::BuildAnsiFrame() const
    {
        std::wstring frame = L"\x1b[H";
        frame.reserve(kScreenWidth * kScreenHeight + 2048);
        int currentColorCode = -1;

        // 색상이 달라지는 지점에만 ANSI 코드를 넣어 프레임 문자열 크기를 줄인다.
        for (int y = 0; y < kScreenHeight; ++y)
        {
            for (int x = 0; x < kScreenWidth; ++x)
            {
                const Cell& cell = cells_[y * kScreenWidth + x];
                if (cell.isWideContinuation)
                {
                    continue;
                }

                const int colorCode = static_cast<int>(cell.color);
                if (colorCode != currentColorCode)
                {
                    frame += L"\x1b[";
                    frame += std::to_wstring(colorCode);
                    frame += L"m";
                    currentColorCode = colorCode;
                }
                frame += cell.glyph;
            }

            if (y != kScreenHeight - 1)
            {
                frame += L'\n';
            }
        }

        frame += L"\x1b[0m";
        return frame;
    }

    bool ScreenBuffer::IsInside(int x, int y) const noexcept
    {
        return x >= 0 && x < kScreenWidth && y >= 0 && y < kScreenHeight;
    }

    int ScreenBuffer::GetDisplayWidth(wchar_t glyph) noexcept
    {
        // Windows 콘솔에서 두 칸을 차지하는 한글·CJK·전각 문자의 주요 Unicode 범위다.
        const bool isWide =
            (glyph >= L'\x1100' && glyph <= L'\x115F') ||
            (glyph >= L'\x2E80' && glyph <= L'\xA4CF') ||
            (glyph >= L'\xAC00' && glyph <= L'\xD7A3') ||
            (glyph >= L'\xF900' && glyph <= L'\xFAFF') ||
            (glyph >= L'\xFE10' && glyph <= L'\xFE6F') ||
            (glyph >= L'\xFF01' && glyph <= L'\xFF60') ||
            (glyph >= L'\xFFE0' && glyph <= L'\xFFE6');
        return isWide ? 2 : 1;
    }

    int ScreenBuffer::GetDisplayWidth(std::wstring_view text) noexcept
    {
        int width = 0;
        for (const wchar_t glyph : text)
        {
            width += GetDisplayWidth(glyph);
        }
        return width;
    }

    void ScreenBuffer::ClearOccupiedCell(int x, int y)
    {
        if (!IsInside(x, y))
        {
            return;
        }

        const int index = y * kScreenWidth + x;
        if (cells_[index].isWideContinuation && x > 0)
        {
            cells_[index - 1] = {};
        }
        else if (GetDisplayWidth(cells_[index].glyph) == 2 &&
                 IsInside(x + 1, y) &&
                 cells_[index + 1].isWideContinuation)
        {
            cells_[index + 1] = {};
        }
        cells_[index] = {};
    }
}
