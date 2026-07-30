#pragma once

#include "Platform/IInput.h"

#include <array>
#include <string>

namespace ss
{
    /// Windows 키 상태를 게임 입력 값으로 변환한다.
    class ConsoleInput final : public IInput
    {
    public:
        void Update() override;
        [[nodiscard]] bool IsDown(InputKey key) const override;
        [[nodiscard]] bool WasPressed(InputKey key) const override;
        [[nodiscard]] std::wstring_view GetTextInput() const noexcept override;

    private:
        void CollectTextInput();
        [[nodiscard]] static int ToVirtualKey(InputKey key) noexcept;
        [[nodiscard]] static std::size_t ToIndex(InputKey key) noexcept;

        // 눌림 순간을 판정하기 위한 현재 프레임과 이전 프레임 상태다.
        std::array<bool, static_cast<std::size_t>(InputKey::Count)> current_{};
        std::array<bool, static_cast<std::size_t>(InputKey::Count)> previous_{};
        std::wstring textInput_;
    };
}
