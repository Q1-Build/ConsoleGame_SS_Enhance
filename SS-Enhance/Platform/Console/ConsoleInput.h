#pragma once

#include "Platform/IInput.h"

#include <array>
#include <string>

namespace ss
{
    /// Windows 키 상태와 IME 완성 문자를 플랫폼 독립 입력 값으로 변환한다.
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

        // 눌림 순간을 판정하는 키 상태와 이번 프레임의 완성 문자열을 보관한다.
        std::array<bool, static_cast<std::size_t>(InputKey::Count)> current_{};
        std::array<bool, static_cast<std::size_t>(InputKey::Count)> previous_{};
        std::wstring textInput_;
    };
}
