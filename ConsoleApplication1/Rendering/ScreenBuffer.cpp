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
        std::fill(cells_.begin(), cells_.end(), Cell{glyph, color});
    }

    void ScreenBuffer::Put(int x, int y, wchar_t glyph, Color color)
    {
        if (!IsInside(x, y))
        {
            return;
        }

        cells_[y * kScreenWidth + x] = {glyph, color};
    }

    void ScreenBuffer::Text(int x, int y, std::wstring_view text, Color color)
    {
        for (std::size_t index = 0; index < text.size(); ++index)
        {
            Put(x + static_cast<int>(index), y, text[index], color);
        }
    }

    void ScreenBuffer::CenterText(int y, std::wstring_view text, Color color)
    {
        const int left = (kScreenWidth - static_cast<int>(text.size())) / 2;
        Text(left, y, text, color);
    }

    void ScreenBuffer::Line(int x1, int y1, int x2, int y2, wchar_t glyph, Color color)
    {
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

        for (int y = 0; y < kScreenHeight; ++y)
        {
            for (int x = 0; x < kScreenWidth; ++x)
            {
                const Cell& cell = cells_[y * kScreenWidth + x];
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
}
