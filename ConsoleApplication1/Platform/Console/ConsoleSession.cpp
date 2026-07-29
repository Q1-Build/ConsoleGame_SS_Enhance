#include "Platform/Console/ConsoleSession.h"

#include <cwchar>

namespace ss
{
    ConsoleSession::ConsoleSession()
        : outputHandle_(GetStdHandle(STD_OUTPUT_HANDLE)),
          inputHandle_(GetStdHandle(STD_INPUT_HANDLE))
    {
        hasOutputMode_ = GetConsoleMode(outputHandle_, &oldOutputMode_) != 0;
        hasInputMode_ = GetConsoleMode(inputHandle_, &oldInputMode_) != 0;

        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);

        if (hasOutputMode_)
        {
            SetConsoleMode(outputHandle_, oldOutputMode_ | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
        if (hasInputMode_)
        {
            const DWORD inputMode = oldInputMode_ &
                ~(ENABLE_QUICK_EDIT_MODE | ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
            SetConsoleMode(inputHandle_, inputMode);
        }

        SetConsoleTitleW(L"SS_Enhance");
        WriteControlSequence(L"\x1b[?1049h\x1b[2J\x1b[?25l");
    }

    ConsoleSession::~ConsoleSession() noexcept
    {
        WriteControlSequence(L"\x1b[0m\x1b[?25h\x1b[?1049l");

        if (hasOutputMode_)
        {
            SetConsoleMode(outputHandle_, oldOutputMode_);
        }
        if (hasInputMode_)
        {
            SetConsoleMode(inputHandle_, oldInputMode_);
        }
    }

    void ConsoleSession::WriteControlSequence(const wchar_t* text) noexcept
    {
        DWORD written = 0;
        WriteConsoleW(
            GetStdHandle(STD_OUTPUT_HANDLE),
            text,
            static_cast<DWORD>(wcslen(text)),
            &written,
            nullptr);
    }
}
