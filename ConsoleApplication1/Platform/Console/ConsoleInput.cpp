#define NOMINMAX
#include "Platform/Console/ConsoleInput.h"

#include <Windows.h>

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
        case InputKey::Left:
            return VK_LEFT;
        case InputKey::Right:
            return VK_RIGHT;
        case InputKey::A:
            return 'A';
        case InputKey::D:
            return 'D';
        case InputKey::Q:
            return 'Q';
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
