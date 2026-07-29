#pragma once

#include "Rendering/IFramePresenter.h"

namespace ss
{
    /// ANSI 프레임을 Windows 콘솔 출력 핸들에 전달한다.
    class ConsolePresenter final : public IFramePresenter
    {
    public:
        void Present(std::wstring_view frame) override;
    };
}
