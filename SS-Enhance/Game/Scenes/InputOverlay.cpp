#include "Game/Scenes/InputOverlay.h"

#include "Core/GameConstants.h"
#include "Game/Scenes/LocalizedText.h"
#include "Platform/IInput.h"
#include "Rendering/IScreen.h"

#include <algorithm>
#include <cassert>

namespace ss
{
    namespace
    {
        constexpr float kPressFlashSeconds = 0.065f;
        constexpr int kGuideLeft = kGameViewportWidth + 1;
        constexpr int kGuideRight = kScreenWidth - 2;
        constexpr int kKeyboardTop = kGameViewportHeight + 1;
        constexpr int kKeyboardRight = kGameViewportWidth - 2;
        constexpr int kKeyboardBottom = kScreenHeight - 1;

        static_assert(kGuideLeft < kGuideRight);
        static_assert(kKeyboardTop < kKeyboardBottom);
    }

    void InputOverlay::Update(float deltaSeconds, const IInput& input)
    {
        assert(deltaSeconds >= 0.0f);
        // 새 입력은 짧게 노란색으로 점멸하고, 계속 누르는 입력은 청록색 상태로 따로 남긴다.
        for (std::size_t index = 0; index < pressFlashSeconds_.size(); ++index)
        {
            const InputKey key = static_cast<InputKey>(index);
            isHeld_[index] = input.IsDown(key);
            if (input.WasPressed(key))
            {
                pressFlashSeconds_[index] = kPressFlashSeconds;
                continue;
            }

            pressFlashSeconds_[index] = std::max(
                0.0f,
                pressFlashSeconds_[index] - deltaSeconds);
        }
    }

    void InputOverlay::Draw(IScreen& screen, Language language) const
    {
        DrawControlGuide(screen, language);
        DrawVirtualKeyboard(screen, language);
    }

    void InputOverlay::DrawControlGuide(
        IScreen& screen,
        Language language) const
    {
        screen.Box(
            kGuideLeft,
            0,
            kGuideRight,
            kGameViewportHeight - 1,
            Color::BrightBlack);
        screen.CenterTextIn(
            kGuideLeft,
            kGuideRight,
            2,
            LocalizedText::Select(
                language,
                L"조 작  안 내",
                L"C O N T R O L S"),
            Color::BrightCyan);
        screen.Line(
            kGuideLeft + 3,
            4,
            kGuideRight - 3,
            4,
            L'─',
            Color::BrightBlack);

        // 장면마다 의미가 조금씩 달라지는 키는 대표 동작을 함께 적어 공통 안내로 유지한다.
        DrawGuideRow(
            screen,
            6,
            L"[ W / ↑ ]",
            LocalizedText::Select(language, L"위 / 메뉴 이동", L"UP / MENU"),
            InputKey::W,
            InputKey::Up);
        DrawGuideRow(
            screen,
            9,
            L"[ S / ↓ ]",
            LocalizedText::Select(language, L"아래 / 메뉴 이동", L"DOWN / MENU"),
            InputKey::S,
            InputKey::Down);
        DrawGuideRow(
            screen,
            12,
            L"[ A / ← ]",
            LocalizedText::Select(
                language,
                L"냉각 / 방어·반격 / 선택",
                L"COOL / GUARD·COUNTER / SELECT"),
            InputKey::A,
            InputKey::Left);
        DrawGuideRow(
            screen,
            15,
            L"[ D / → ]",
            LocalizedText::Select(language, L"가열 / 선택", L"STOKE / SELECT"),
            InputKey::D,
            InputKey::Right);
        DrawGuideRow(
            screen,
            18,
            L"[ SPACE ]",
            LocalizedText::Select(language, L"타격 / 공격", L"STRIKE / ATTACK"),
            InputKey::Space,
            InputKey::Space);
        DrawGuideRow(
            screen,
            21,
            L"[ ENTER ]",
            LocalizedText::Select(language, L"확인 / 진행", L"CONFIRM / CONTINUE"),
            InputKey::Enter,
            InputKey::Enter);
        DrawGuideRow(
            screen,
            24,
            L"[ B ]",
            LocalizedText::Select(language, L"보스 도전", L"CHALLENGE BOSS"),
            InputKey::B,
            InputKey::B);
        DrawGuideRow(
            screen,
            27,
            L"[ Q ]",
            LocalizedText::Select(language, L"대장간 종료", L"LEAVE FORGE"),
            InputKey::Q,
            InputKey::Q);
        DrawGuideRow(
            screen,
            30,
            L"[ ESC ]",
            LocalizedText::Select(language, L"뒤로 / 후퇴", L"BACK / RETREAT"),
            InputKey::Escape,
            InputKey::Escape);
    }

    void InputOverlay::DrawVirtualKeyboard(
        IScreen& screen,
        Language language) const
    {
        screen.Box(
            1,
            kKeyboardTop,
            kKeyboardRight,
            kKeyboardBottom,
            Color::BrightBlack);
        screen.Text(
            4,
            36,
            LocalizedText::Select(
                language,
                L"실시간 입력",
                L"LIVE INPUT"),
            Color::BrightCyan);
        screen.RightText(
            99,
            36,
            LocalizedText::Select(
                language,
                L"A 반격   Q / ESC 나가기",
                L"A COUNTER   Q / ESC EXIT"),
            Color::BrightBlack);

        // 문자 이동키와 방향키를 같은 모양으로 배치해 두 조작 방식의 대응 관계를 보여 준다.
        DrawKey(screen, 14, 38, 22, 40, L"W", InputKey::W);
        DrawKey(screen, 5, 42, 13, 44, L"A", InputKey::A);
        DrawKey(screen, 14, 42, 22, 44, L"S", InputKey::S);
        DrawKey(screen, 23, 42, 31, 44, L"D", InputKey::D);

        DrawKey(screen, 50, 38, 58, 40, L"↑", InputKey::Up);
        DrawKey(screen, 41, 42, 49, 44, L"←", InputKey::Left);
        DrawKey(screen, 50, 42, 58, 44, L"↓", InputKey::Down);
        DrawKey(screen, 59, 42, 67, 44, L"→", InputKey::Right);

        DrawKey(screen, 70, 38, 82, 40, L"ENTER", InputKey::Enter);
        DrawKey(screen, 84, 38, 90, 40, L"Q", InputKey::Q);
        DrawKey(screen, 92, 38, 100, 40, L"ESC", InputKey::Escape);
        DrawKey(screen, 70, 42, 100, 44, L"SPACE", InputKey::Space);

        screen.CenterTextIn(
            5,
            31,
            46,
            LocalizedText::Select(language, L"WASD · A 반격", L"WASD · A COUNTER"),
            Color::BrightBlack);
        screen.CenterTextIn(
            41,
            67,
            46,
            LocalizedText::Select(language, L"방향키", L"ARROWS"),
            Color::BrightBlack);
        screen.CenterTextIn(
            70,
            100,
            46,
            LocalizedText::Select(
                language,
                L"확인 / 행동 / 나가기",
                L"ACTION / EXIT"),
            Color::BrightBlack);
    }

    void InputOverlay::DrawKey(
        IScreen& screen,
        int left,
        int top,
        int right,
        int bottom,
        std::wstring_view label,
        InputKey key) const
    {
        const KeyVisualState state = GetVisualState(key);
        const Color borderColor = GetHighlightColor(state);

        // 채우기보다 테두리와 글자를 밝히면 작은 키 캡도 원래 문자 형태를 잃지 않는다.
        screen.Box(left, top, right, bottom, borderColor);
        screen.CenterTextIn(
            left,
            right,
            top + 1,
            label,
            state == KeyVisualState::Idle
                ? Color::White
                : Color::BrightWhite);
    }

    void InputOverlay::DrawGuideRow(
        IScreen& screen,
        int y,
        std::wstring_view keys,
        std::wstring_view description,
        InputKey primaryKey,
        InputKey secondaryKey) const
    {
        const KeyVisualState state =
            GetVisualState(primaryKey, secondaryKey);
        const Color highlightColor = GetHighlightColor(state);
        screen.Text(
            kGuideLeft + 4,
            y,
            keys,
            state == KeyVisualState::Idle
                ? Color::BrightWhite
                : highlightColor);
        screen.Text(
            kGuideLeft + 18,
            y,
            description,
            state == KeyVisualState::Idle
                ? Color::BrightBlack
                : Color::BrightWhite);
    }

    InputOverlay::KeyVisualState InputOverlay::GetVisualState(
        InputKey key) const noexcept
    {
        const std::size_t index = static_cast<std::size_t>(key);
        assert(index < pressFlashSeconds_.size());
        if (pressFlashSeconds_[index] > 0.0f)
        {
            return KeyVisualState::Pressed;
        }
        return isHeld_[index]
            ? KeyVisualState::Held
            : KeyVisualState::Idle;
    }

    InputOverlay::KeyVisualState InputOverlay::GetVisualState(
        InputKey primaryKey,
        InputKey secondaryKey) const noexcept
    {
        const KeyVisualState primaryState = GetVisualState(primaryKey);
        const KeyVisualState secondaryState = GetVisualState(secondaryKey);
        if (primaryState == KeyVisualState::Pressed ||
            secondaryState == KeyVisualState::Pressed)
        {
            return KeyVisualState::Pressed;
        }
        if (primaryState == KeyVisualState::Held ||
            secondaryState == KeyVisualState::Held)
        {
            return KeyVisualState::Held;
        }
        return KeyVisualState::Idle;
    }

    Color InputOverlay::GetHighlightColor(
        KeyVisualState state) noexcept
    {
        switch (state)
        {
        case KeyVisualState::Pressed:
            return Color::BrightYellow;
        case KeyVisualState::Held:
            return Color::BrightCyan;
        case KeyVisualState::Idle:
            return Color::BrightBlack;
        }
        return Color::BrightBlack;
    }
}
