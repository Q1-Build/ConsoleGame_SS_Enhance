#include "Game/Scenes/GameHudRenderer.h"

#include "Core/GameConstants.h"
#include "Game/Domain/Difficulty.h"
#include "Game/Domain/PlayerProgress.h"
#include "Game/Scenes/LocalizedText.h"
#include "Rendering/IScreen.h"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace ss
{
    namespace
    {
        Color GetDifficultyColor(Difficulty difficulty) noexcept
        {
            switch (difficulty)
            {
            case Difficulty::Easy:
                return Color::BrightGreen;
            case Difficulty::Normal:
                return Color::BrightCyan;
            case Difficulty::Hard:
                return Color::BrightRed;
            }
            return Color::BrightWhite;
        }
    }

    void GameHudRenderer::DrawBackdrop(IScreen& screen, float worldTimeSeconds) const
    {
        screen.Clear();

        // 모든 장면이 같은 크기와 색상의 외곽 프레임을 공유한다.
        for (int y = 1; y < kGameViewportHeight - 1; ++y)
        {
            screen.Put(1, y, L'│', Color::BrightBlack);
            screen.Put(kGameViewportWidth - 2, y, L'│', Color::BrightBlack);
        }
        for (int x = 2; x < kGameViewportWidth - 2; ++x)
        {
            screen.Put(x, 0, L'─', Color::BrightBlack);
            screen.Put(x, kGameViewportHeight - 1, L'─', Color::BrightBlack);
        }

        screen.Put(1, 0, L'┌', Color::BrightBlack);
        screen.Put(kGameViewportWidth - 2, 0, L'┐', Color::BrightBlack);
        screen.Put(1, kGameViewportHeight - 1, L'└', Color::BrightBlack);
        screen.Put(
            kGameViewportWidth - 2,
            kGameViewportHeight - 1,
            L'┘',
            Color::BrightBlack);

        // 사인 파형으로 하단 화로의 밝은 범위를 천천히 넓혔다 줄인다.
        const int pulse = static_cast<int>(
            (std::sin(worldTimeSeconds * 1.7f) + 1.0f) * 0.5f * 12.0f);
        for (int x = 3; x < kGameViewportWidth - 3; ++x)
        {
            const int distance = std::abs(x - kGameViewportWidth / 2);
            const int animationFrame = static_cast<int>(worldTimeSeconds * 7.0f);
            if (distance < 14 + pulse && (x + animationFrame) % 3 == 0)
            {
                const Color color = distance < 8 ? Color::BrightRed : Color::Red;
                screen.Put(x, kGameViewportHeight - 2, L'▁', color);
            }
        }
    }

    void GameHudRenderer::DrawHeader(
        IScreen& screen,
        const PlayerProgress& progress,
        Language language,
        Difficulty difficulty) const
    {
        constexpr std::wstring_view title = L"S S _ E N H A N C E";
        screen.Text(4, 2, title, Color::BrightRed);

        // 로고 옆의 고정 배지로 플레이 중인 규칙 난이도를 항상 확인할 수 있게 한다.
        std::wstringstream difficultyBadge;
        difficultyBadge
            << LocalizedText::Select(language, L"난이도 ", L"DIFFICULTY ")
            << L"[ "
            << LocalizedText::GetDifficultyName(language, difficulty)
            << L" ]";
        screen.Text(
            4 + screen.MeasureText(title) + 4,
            2,
            difficultyBadge.str(),
            GetDifficultyColor(difficulty));

        std::wstringstream status;
        status << LocalizedText::Select(language, L"골드 ", L"GOLD ")
               << progress.GetGold()
               << LocalizedText::Select(language, L" G   기억 조각 ", L" G   SHARDS ")
               << progress.GetFragments();
        const std::wstring statusText = status.str();
        screen.RightText(
            kGameViewportWidth - 5,
            2,
            statusText,
            Color::BrightYellow);
        screen.Line(
            4,
            3,
            kGameViewportWidth - 5,
            3,
            L'─',
            Color::BrightBlack);
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

        // +2부터 검 주변에 움직이는 오라를 추가하고 단계에 따라 검신을 길게 만든다.
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
        // 호출자가 범위를 벗어난 값을 전달해도 막대가 프레임을 침범하지 않게 보정한다.
        const float safeValue = Clamp01(value);
        const int labelWidth = screen.MeasureText(label);
        screen.Text(x, y, label, Color::BrightWhite);
        screen.Put(x + labelWidth, y, L'[', Color::BrightBlack);

        const int filled = static_cast<int>(safeValue * static_cast<float>(width));
        for (int index = 0; index < width; ++index)
        {
            screen.Put(
                x + labelWidth + 1 + index,
                y,
                index < filled ? L'█' : L'░',
                index < filled ? fillColor : Color::BrightBlack);
        }

        screen.Put(
            x + labelWidth + width + 1,
            y,
            L']',
            Color::BrightBlack);
    }

    Color GameHudRenderer::GetSwordColor(int swordLevel) const noexcept
    {
        // 색상 등급은 화면 표현 규칙이므로 도메인 Sword가 아니라 HUD에서 결정한다.
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
