#include "Platform/Console/ConsolePresenter.h"

#define NOMINMAX
#include <Windows.h>

namespace ss
{
    void ConsolePresenter::Present(std::wstring_view frame)
    {
        // 완성된 프레임을 한 번의 시스템 호출로 출력해 프레임 중간 깜빡임을 줄인다.
        DWORD written = 0;
        WriteConsoleW(
            GetStdHandle(STD_OUTPUT_HANDLE),
            frame.data(),
            static_cast<DWORD>(frame.size()),
            &written,
            nullptr);
    }
}
