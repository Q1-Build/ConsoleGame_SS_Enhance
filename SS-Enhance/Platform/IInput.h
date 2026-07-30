#pragma once

#include "Platform/InputKey.h"

#include <string_view>

namespace ss
{
    /// 프레임 단위 키 상태와 완성된 문자 입력을 제공하는 플랫폼 독립 계약이다.
    class IInput
    {
    public:
        virtual ~IInput() = default;

        /// 새 프레임의 키 상태를 수집하고 이전 상태를 보존한다.
        virtual void Update() = 0;

        /// 지정한 키가 현재 계속 눌려 있는지 반환한다.
        [[nodiscard]] virtual bool IsDown(InputKey key) const = 0;

        /// 지정한 키가 이번 프레임에 처음 눌렸는지 반환한다.
        [[nodiscard]] virtual bool WasPressed(InputKey key) const = 0;

        /// 이번 프레임에 플랫폼 입력기로 완성된 인쇄 가능 문자열을 반환한다.
        [[nodiscard]] virtual std::wstring_view GetTextInput() const noexcept = 0;
    };
}
