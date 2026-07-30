#include "Game/Scenes/PresentationChatOverlay.h"

#include "Core/GameConstants.h"
#include "Game/Scenes/LocalizedText.h"
#include "Platform/IInput.h"
#include "Rendering/IScreen.h"

#include <algorithm>
#include <string_view>
#include <utility>

namespace ss
{
    namespace
    {
        constexpr int kChatLeft = kGameViewportWidth + 1;
        constexpr int kChatTop = kGameViewportHeight + 1;
        constexpr int kChatRight = kScreenWidth - 2;
        constexpr int kChatBottom = kScreenHeight - 1;
        constexpr int kMessageTop = kChatTop + 3;
        constexpr int kMessageBottom = kChatBottom - 4;
        constexpr int kVisibleLineCount = kMessageBottom - kMessageTop + 1;
        constexpr int kContentWidth = kChatRight - kChatLeft - 6;
        constexpr std::size_t kMaximumInputCharacters = 160;

        static_assert(kVisibleLineCount > 0);
        static_assert(kContentWidth > 0);
    }

    bool PresentationChatOverlay::Update(
        const IInput& input,
        const IScreen& screen)
    {
        if (!isEditing_)
        {
            const std::wstring_view textInput = input.GetTextInput();
            const bool typedChatKey =
                textInput.find(L't') != std::wstring_view::npos ||
                textInput.find(L'T') != std::wstring_view::npos;
            if (!input.WasPressed(InputKey::T) && !typedChatKey)
            {
                return false;
            }

            // 채팅을 연 T가 같은 프레임의 메시지 첫 글자로 들어가지 않게 편집을 다음 프레임부터 시작한다.
            currentInput_.clear();
            isEditing_ = true;
            return true;
        }

        if (input.WasPressed(InputKey::Escape))
        {
            currentInput_.clear();
            isEditing_ = false;
            return true;
        }
        if (input.WasPressed(InputKey::Enter))
        {
            SubmitCurrentInput(screen);
            currentInput_.clear();
            isEditing_ = false;
            return true;
        }
        if (input.WasPressed(InputKey::Backspace) && !currentInput_.empty())
        {
            currentInput_.pop_back();
        }

        const std::wstring_view textInput = input.GetTextInput();
        const std::size_t remainingCharacters =
            kMaximumInputCharacters - currentInput_.size();
        currentInput_.append(
            textInput.substr(0, std::min(remainingCharacters, textInput.size())));
        return true;
    }

    void PresentationChatOverlay::Draw(
        IScreen& screen,
        Language language) const
    {
        screen.Box(
            kChatLeft,
            kChatTop,
            kChatRight,
            kChatBottom,
            isEditing_ ? Color::BrightCyan : Color::BrightBlack);
        screen.Text(
            kChatLeft + 3,
            kChatTop + 1,
            LocalizedText::Select(
                language,
                L"채 팅  박 스",
                L"CHAT BOX"),
            Color::BrightCyan);
        screen.RightText(
            kChatRight - 3,
            kChatTop + 1,
            isEditing_
                ? LocalizedText::Select(
                    language,
                    L"[ ENTER ] 등록",
                    L"[ ENTER ] POST")
                : LocalizedText::Select(
                    language,
                    L"[ T ] 입력",
                    L"[ T ] TYPE"),
            isEditing_ ? Color::BrightYellow : Color::BrightBlack);
        screen.Line(
            kChatLeft + 2,
            kChatTop + 2,
            kChatRight - 2,
            kChatTop + 2,
            L'─',
            Color::BrightBlack);

        // 최신 줄은 아래에 두고 새 메시지가 들어오면 기존 줄을 한 칸씩 위로 밀어 채팅 흐름을 만든다.
        const int firstLineY =
            kMessageBottom - static_cast<int>(lines_.size()) + 1;
        for (std::size_t index = 0; index < lines_.size(); ++index)
        {
            screen.Text(
                kChatLeft + 3,
                firstLineY + static_cast<int>(index),
                lines_[index],
                Color::BrightWhite);
        }

        screen.Line(
            kChatLeft + 2,
            kChatBottom - 3,
            kChatRight - 2,
            kChatBottom - 3,
            L'─',
            Color::BrightBlack);
        if (isEditing_)
        {
            screen.Text(kChatLeft + 3, kChatBottom - 2, L">", Color::BrightCyan);
            const std::wstring visibleInput = GetVisibleInput(screen);
            screen.Text(
                kChatLeft + 5,
                kChatBottom - 2,
                visibleInput,
                Color::BrightWhite);
            const int cursorX = std::min(
                kChatRight - 2,
                kChatLeft + 5 + screen.MeasureText(visibleInput));
            screen.Put(cursorX, kChatBottom - 2, L'▌', Color::BrightCyan);
            screen.Text(
                kChatLeft + 3,
                kChatBottom - 1,
                LocalizedText::Select(
                    language,
                    L"게임 일시 정지 · ESC 취소",
                    L"GAME PAUSED · ESC CANCEL"),
                Color::BrightBlack);
            return;
        }

        screen.Text(
            kChatLeft + 3,
            kChatBottom - 2,
            LocalizedText::Select(
                language,
                L"T를 눌러 언제든 메시지를 입력하세요.",
                L"PRESS T TO TYPE A MESSAGE."),
            Color::BrightBlack);
    }

    void PresentationChatOverlay::SubmitCurrentInput(const IScreen& screen)
    {
        const std::size_t firstCharacter =
            currentInput_.find_first_not_of(L" \t");
        if (firstCharacter == std::wstring::npos)
        {
            return;
        }
        const std::size_t lastCharacter =
            currentInput_.find_last_not_of(L" \t");
        const std::wstring message = currentInput_.substr(
            firstCharacter,
            lastCharacter - firstCharacter + 1);

        std::wstring line;
        for (const wchar_t glyph : message)
        {
            std::wstring candidate = line;
            candidate += glyph;
            if (!line.empty() && screen.MeasureText(candidate) > kContentWidth)
            {
                lines_.push_back(line);
                line.clear();
            }
            line += glyph;
        }
        if (!line.empty())
        {
            lines_.push_back(line);
        }

        while (lines_.size() > static_cast<std::size_t>(kVisibleLineCount))
        {
            lines_.pop_front();
        }
    }

    std::wstring PresentationChatOverlay::GetVisibleInput(
        const IScreen& screen) const
    {
        std::wstring visibleInput;
        for (auto iterator = currentInput_.rbegin();
             iterator != currentInput_.rend();
             ++iterator)
        {
            std::wstring candidate(1, *iterator);
            candidate += visibleInput;
            if (screen.MeasureText(candidate) > kContentWidth)
            {
                break;
            }
            visibleInput = std::move(candidate);
        }
        return visibleInput;
    }
}
