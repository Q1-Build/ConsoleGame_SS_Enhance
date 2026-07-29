#define NOMINMAX
#include "Platform/Console/ConsolePresenter.h"

#include <Windows.h>

namespace ss
{
    void ConsolePresenter::Present(std::wstring_view frame)
    {
        DWORD written = 0;
        WriteConsoleW(
            GetStdHandle(STD_OUTPUT_HANDLE),
            frame.data(),
            static_cast<DWORD>(frame.size()),
            &written,
            nullptr);
    }
}
