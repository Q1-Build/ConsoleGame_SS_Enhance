#include "Game/Scenes/GameHudRenderer.h"

#include "Core/GameConstants.h"
#include "Game/Domain/PlayerProgress.h"
#include "Rendering/IScreen.h"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace ss
{
    void GameHudRenderer::DrawBackdrop(IScreen& screen, float worldTimeSeconds) const
    {
        screen.Clear();

        for (int y = 1; y < kScreenHeight - 1; ++y)
        {
            screen.Put(1, y, L'│', Color::BrightBlack);
            screen.Put(kScreenWidth - 2, y, L'│', Color::BrightBlack);
        }
        for (int x = 2; x < kScreenWidth - 2; ++x)
        {
            screen.Put(x, 0, L'─', Color::BrightBlack);
            screen.Put(x, kScreenHeight - 1, L'─', Color::BrightBlack);
        }

        screen.Put(1, 0, L'┌', Color::BrightBlack);
        screen.Put(kScreenWidth - 2, 0, L'┐', Color::BrightBlack);
        screen.Put(1, kScreenHeight - 1, L'└', Color::BrightBlack);
        screen.Put(kScreenWidth - 2, kScreenHeight - 1, L'┘', Color::BrightBlack);

        const int pulse = static_cast<int>(
            (std::sin(worldTimeSeconds * 1.7f) + 1.0f) * 0.5f * 12.0f);
        for (int x = 3; x < kScreenWidth - 3; ++x)
        {
            const int distance = std::abs(x - kScreenWidth / 2);
            const int animationFrame = static_cast<int>(worldTimeSeconds * 7.0f);
            if (distance < 14 + pulse && (x + animationFrame) % 3 == 0)
            {
                const Color color = distance < 8 ? Color::BrightRed : Color::Red;
                screen.Put(x, kScreenHeight - 2, L'▁', color);
            }
        }
    }

    void GameHudRenderer::DrawHeader(IScreen& screen, const PlayerProgress& progress) const
    {
        screen.Text(4, 2, L"S S _ E N H A N C E", Color::BrightRed);

        std::wstringstream status;
        status << L"GOLD " << progress.GetGold()
               << L" G   SHARDS " << progress.GetFragments();
        const std::wstring statusText = status.str();
        screen.Text(
            kScreenWidth - 5 - static_cast<int>(statusText.size()),
            2,
            statusText,
            Color::BrightYellow);
        screen.Line(4, 3, kScreenWidth - 5, 3, L'─', Color::BrightBlack);
    }

    void GameHudRenderer::DrawSword(
        IScreen& screen,
        int centerX,
        int topY,
        Color color,
        int level,
        float glow,
        float worldTimeSeconds) const
    {
        const Color auraColor = glow > 0.52f ? color : Color::BrightBlack;
        const int length = 11 + std::min(level / 2, 4);

        if (level >= 2)
        {
            for (int index = 0; index < length; ++index)
            {
                const int animationFrame = static_cast<int>(worldTimeSeconds * 9.0f);
                if ((index + animationFrame) % 3 == 0)
                {
                    screen.Put(centerX - 2, topY + index, L'·', auraColor);
                    screen.Put(centerX + 2, topY + index, L'·', auraColor);
                }
            }
        }

        screen.Put(
            centerX,
            topY - 1,
            L'✦',
            level >= 6 ? color : Color::BrightWhite);
        screen.Put(centerX, topY, L'▲', color);
        for (int index = 1; index < length; ++index)
        {
            screen.Put(centerX - 1, topY + index, L'╱', color);
            screen.Put(
                centerX,
                topY + index,
                level >= 8 ? L'║' : L'│',
                Color::BrightWhite);
            screen.Put(centerX + 1, topY + index, L'╲', color);
        }

        screen.Put(centerX - 4, topY + length, L'═', Color::BrightYellow);
        screen.Put(centerX - 3, topY + length, L'═', Color::BrightYellow);
        screen.Put(centerX - 2, topY + length, L'╪', Color::BrightYellow);
        screen.Put(centerX - 1, topY + length, L'╪', Color::BrightYellow);
        screen.Put(centerX, topY + length, L'╬', Color::BrightWhite);
        screen.Put(centerX + 1, topY + length, L'╪', Color::BrightYellow);
        screen.Put(centerX + 2, topY + length, L'╪', Color::BrightYellow);
        screen.Put(centerX + 3, topY + length, L'═', Color::BrightYellow);
        screen.Put(centerX + 4, topY + length, L'═', Color::BrightYellow);
        screen.Put(centerX, topY + length + 1, L'║', Color::Yellow);
        screen.Put(centerX, topY + length + 2, L'║', Color::Yellow);
        screen.Put(centerX, topY + length + 3, L'◆', Color::BrightRed);
    }

    void GameHudRenderer::DrawProgressBar(
        IScreen& screen,
        int x,
        int y,
        int width,
        float value,
        Color fillColor,
        std::wstring_view label) const
    {
        const float safeValue = Clamp01(value);
        screen.Text(x, y, label, Color::BrightWhite);
        screen.Put(x + static_cast<int>(label.size()), y, L'[', Color::BrightBlack);

        const int filled = static_cast<int>(safeValue * static_cast<float>(width));
        for (int index = 0; index < width; ++index)
        {
            screen.Put(
                x + static_cast<int>(label.size()) + 1 + index,
                y,
                index < filled ? L'█' : L'░',
                index < filled ? fillColor : Color::BrightBlack);
        }

        screen.Put(
            x + static_cast<int>(label.size()) + width + 1,
            y,
            L']',
            Color::BrightBlack);
    }

    Color GameHudRenderer::GetSwordColor(int swordLevel) const noexcept
    {
        if (swordLevel >= 10)
        {
            return Color::BrightMagenta;
        }
        if (swordLevel >= 8)
        {
            return Color::BrightCyan;
        }
        if (swordLevel >= 6)
        {
            return Color::BrightBlue;
        }
        if (swordLevel >= 4)
        {
            return Color::BrightRed;
        }
        if (swordLevel >= 2)
        {
            return Color::BrightYellow;
        }
        return Color::BrightWhite;
    }
}
