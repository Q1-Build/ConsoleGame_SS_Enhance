#define NOMINMAX
#include "Platform/Console/ConsoleInput.h"

#include <Windows.h>

#include <algorithm>
#include <array>
#include <cassert>

namespace ss
{
    void ConsoleInput::Update()
    {
        // 이전 배열을 먼저 보존해야 Down 상태와 이번 프레임의 Pressed 상태를 함께 계산할 수 있다.
        previous_ = current_;
        for (std::size_t index = 0; index < current_.size(); ++index)
        {
            const auto key = static_cast<InputKey>(index);
            current_[index] = (GetAsyncKeyState(ToVirtualKey(key)) & 0x8000) != 0;
        }
        CollectTextInput();
    }

    bool ConsoleInput::IsDown(InputKey key) const
    {
        return current_[ToIndex(key)];
    }

    bool ConsoleInput::WasPressed(InputKey key) const
    {
        const std::size_t index = ToIndex(key);
        return current_[index] && !previous_[index];
    }

    std::wstring_view ConsoleInput::GetTextInput() const noexcept
    {
        return textInput_;
    }

    void ConsoleInput::CollectTextInput()
    {
        textInput_.clear();
        const HANDLE inputHandle = GetStdHandle(STD_INPUT_HANDLE);
        if (inputHandle == INVALID_HANDLE_VALUE || inputHandle == nullptr)
        {
            return;
        }

        // 콘솔 문자 이벤트만 추출해 IME가 완성한 한글도 게임 계층에 Unicode 문자열로 전달한다.
        std::array<INPUT_RECORD, 64> records{};
        DWORD pendingCount = 0;
        while (GetNumberOfConsoleInputEvents(inputHandle, &pendingCount) != 0 &&
               pendingCount > 0)
        {
            const DWORD requestedCount = std::min(
                pendingCount,
                static_cast<DWORD>(records.size()));
            DWORD readCount = 0;
            if (ReadConsoleInputW(
                    inputHandle,
                    records.data(),
                    requestedCount,
                    &readCount) == 0)
            {
                break;
            }

            for (DWORD index = 0; index < readCount; ++index)
            {
                const INPUT_RECORD& record = records[index];
                if (record.EventType != KEY_EVENT ||
                    record.Event.KeyEvent.bKeyDown == FALSE)
                {
                    continue;
                }

                const KEY_EVENT_RECORD& keyEvent = record.Event.KeyEvent;
                const wchar_t character = keyEvent.uChar.UnicodeChar;
                if (character < L' ' || character == L'\x7f')
                {
                    continue;
                }
                textInput_.append(keyEvent.wRepeatCount, character);
            }
        }
    }

    int ConsoleInput::ToVirtualKey(InputKey key) noexcept
    {
        // Windows 가상 키 코드는 이 변환 경계 밖의 게임 코드로 노출하지 않는다.
        switch (key)
        {
        case InputKey::Enter:
            return VK_RETURN;
        case InputKey::Space:
            return VK_SPACE;
        case InputKey::Escape:
            return VK_ESCAPE;
        case InputKey::Up:
            return VK_UP;
        case InputKey::Down:
            return VK_DOWN;
        case InputKey::Left:
            return VK_LEFT;
        case InputKey::Right:
            return VK_RIGHT;
        case InputKey::W:
            return 'W';
        case InputKey::S:
            return 'S';
        case InputKey::A:
            return 'A';
        case InputKey::D:
            return 'D';
        case InputKey::T:
            return 'T';
        case InputKey::B:
            return 'B';
        case InputKey::Q:
            return 'Q';
        case InputKey::Backspace:
            return VK_BACK;
        case InputKey::Count:
            break;
        }

        assert(false && "지원하지 않는 입력 키다.");
        return 0;
    }

    std::size_t ConsoleInput::ToIndex(InputKey key) noexcept
    {
        const auto index = static_cast<std::size_t>(key);
        assert(index < static_cast<std::size_t>(InputKey::Count));
        return index;
    }
}
