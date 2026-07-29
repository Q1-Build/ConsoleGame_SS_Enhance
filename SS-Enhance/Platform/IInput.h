#pragma once

#include "Platform/InputKey.h"

namespace ss
{
    /// 프레임 단위 키 상태를 제공하는 입력 계약이다.
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
    };
}
