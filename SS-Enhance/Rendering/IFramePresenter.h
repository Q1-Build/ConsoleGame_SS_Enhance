#pragma once

#include <string_view>

namespace ss
{
    /// 완성된 프레임 문자열을 실제 출력 장치에 전달하는 계약이다.
    class IFramePresenter
    {
    public:
        virtual ~IFramePresenter() = default;

        /// 호출 시점까지 완성된 프레임을 소유하지 않는 문자열 뷰로 한 번 출력한다.
        virtual void Present(std::wstring_view frame) = 0;
    };
}
