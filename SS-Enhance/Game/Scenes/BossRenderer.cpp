#include "Game/Scenes/BossRenderer.h"

#include "Rendering/IScreen.h"

#include <algorithm>

namespace ss
{
    void BossRenderer::Draw(
        IScreen& screen,
        BossType bossType,
        float introductionProgress,
        float defeatProgress,
        int shakeX,
        float worldTimeSeconds) const
    {
        // 프레임 시간 초과가 연출 좌표와 색상 범위를 뒤집지 않도록 진행률을 먼저 보정한다.
        const float safeIntro = std::clamp(introductionProgress, 0.0f, 1.0f);
        const float safeDefeat = std::clamp(defeatProgress, 0.0f, 1.0f);
        Color primaryColor = GetPrimaryColor(bossType);
        if (safeIntro < 0.35f || safeDefeat > 0.65f)
        {
            primaryColor = Color::BrightBlack;
        }

        // 격파가 진행되면 온전한 형상 대신 흩어지는 파편을 남겨 승리 판정을 시각화한다.
        if (safeDefeat > 0.25f)
        {
            const int spread = 1 + static_cast<int>(safeDefeat * 5.0f);
            screen.Text(44 - spread + shakeX, 14, L"✦   ·       *", primaryColor);
            screen.Text(47 + spread + shakeX, 16, L"·    ✦", Color::BrightBlack);
            screen.Text(42 - spread + shakeX, 18, L"*       ·", primaryColor);
            return;
        }

        const int pulse =
            static_cast<int>(worldTimeSeconds * 6.0f) % 2 == 0 ? 0 : 1;
        const Color accentColor = pulse == 0
            ? Color::BrightYellow
            : primaryColor;

        // 보스별 실루엣을 완전히 다르게 구성해 색상을 보지 못해도 상대를 구분할 수 있게 한다.
        switch (bossType)
        {
        case BossType::EmberWarden:
            screen.Text(43 + shakeX, 13, L"      ▄████▄      ", primaryColor);
            screen.Text(43 + shakeX, 14, L"   ╔══╬████╬══╗   ", primaryColor);
            screen.Text(43 + shakeX, 15, L"   ║  ◈ ██ ◈  ║   ", accentColor);
            screen.Text(43 + shakeX, 16, L"   ╚══╗████╔══╝   ", primaryColor);
            screen.Text(43 + shakeX, 17, L"      ╚╦══╦╝      ", Color::BrightBlack);
            break;
        case BossType::StormSentinel:
            screen.Text(43 + shakeX, 13, L"    ╲   │   ╱     ", Color::BrightCyan);
            screen.Text(43 + shakeX, 14, L"  ───╲ ϟ ╱───    ", primaryColor);
            screen.Text(43 + shakeX, 15, L"      ╲◇╱        ", accentColor);
            screen.Text(43 + shakeX, 16, L"    ──╱ ╲──      ", primaryColor);
            screen.Text(43 + shakeX, 17, L"    ╱   │  ╲     ", Color::BrightCyan);
            break;
        case BossType::MemoryDevourer:
            screen.Text(43 + shakeX, 13, L"   ╭┄╮       ╭┄╮ ", Color::BrightMagenta);
            screen.Text(43 + shakeX, 14, L" ╭─╯ ◇ ╲   ╱ ◇ ╰─╮", primaryColor);
            screen.Text(43 + shakeX, 15, L" │   ╲  ▒▒▒  ╱   │", accentColor);
            screen.Text(43 + shakeX, 16, L" ╰─╮  ╲ ▒ ╱  ╭─╯ ", primaryColor);
            screen.Text(43 + shakeX, 17, L"   ╰┄┄╲╱ ╲╱┄┄╯   ", Color::BrightBlack);
            break;
        }
    }

    Color BossRenderer::GetPrimaryColor(BossType bossType) const noexcept
    {
        switch (bossType)
        {
        case BossType::EmberWarden:
            return Color::BrightRed;
        case BossType::StormSentinel:
            return Color::BrightCyan;
        case BossType::MemoryDevourer:
            return Color::BrightMagenta;
        }
        return Color::BrightWhite;
    }
}
