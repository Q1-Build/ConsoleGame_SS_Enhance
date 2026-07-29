#pragma once

#include "Platform/InputKey.h"

namespace ss
{
    /// 프레임 단위 키 상태를 제공하는 입력 계약이다.
    class IInput
    {
    public:
        virtual ~IInput() = default;

        virtual void Update() = 0;
        [[nodiscard]] virtual bool IsDown(InputKey key) const = 0;
        [[nodiscard]] virtual bool WasPressed(InputKey key) const = 0;
    };
}
