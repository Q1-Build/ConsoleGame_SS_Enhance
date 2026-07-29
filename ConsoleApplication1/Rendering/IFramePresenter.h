#pragma once

#include <string_view>

namespace ss
{
    /// 완성된 프레임 문자열을 실제 출력 장치에 전달하는 계약이다.
    class IFramePresenter
    {
    public:
        virtual ~IFramePresenter() = default;

        virtual void Present(std::wstring_view frame) = 0;
    };
}
