#include "Platform/Console/ConsoleSession.h"

#include <array>
#include <cwchar>

namespace ss
{
    ConsoleSession::ConsoleSession()
        : outputHandle_(GetStdHandle(STD_OUTPUT_HANDLE)),
          inputHandle_(GetStdHandle(STD_INPUT_HANDLE))
    {
        hasOutputMode_ = GetConsoleMode(outputHandle_, &oldOutputMode_) != 0;
        hasInputMode_ = GetConsoleMode(inputHandle_, &oldInputMode_) != 0;

        if (hasOutputMode_)
        {
            SetConsoleMode(outputHandle_, oldOutputMode_ | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
        if (hasInputMode_)
        {
            // Rider 실행 창처럼 가상 터미널 입력이 활성화된 호스트에서는 마우스 동작이
            // "[M..." 형태의 문자 시퀀스로 변환될 수 있으므로 레코드 기반 입력만 사용한다.
            const DWORD inputMode =
                (oldInputMode_ | ENABLE_EXTENDED_FLAGS) &
                ~(ENABLE_QUICK_EDIT_MODE |
                  ENABLE_LINE_INPUT |
                  ENABLE_ECHO_INPUT |
                  ENABLE_VIRTUAL_TERMINAL_INPUT);
            if (SetConsoleMode(inputHandle_, inputMode) != 0)
            {
                // 실행 호스트가 게임 시작 전에 쌓아 둔 마우스/ANSI 입력이
                // 첫 채팅 내용으로 소비되지 않도록 초기 입력만 폐기한다.
                FlushConsoleInputBuffer(inputHandle_);
            }
        }

        // Wide API만 사용하므로 코드 페이지는 바꾸지 않고, 변경이 필요한 창 제목만 복원용으로 보관한다.
        constexpr std::size_t kMaximumConsoleTitleLength = 32768;
        std::array<wchar_t, kMaximumConsoleTitleLength> titleBuffer{};
        SetLastError(ERROR_SUCCESS);
        const DWORD titleLength = GetConsoleTitleW(
            titleBuffer.data(),
            static_cast<DWORD>(titleBuffer.size()));
        hasOldTitle_ = titleLength > 0 || GetLastError() == ERROR_SUCCESS;
        if (hasOldTitle_)
        {
            oldTitle_.assign(titleBuffer.data(), titleLength);
        }
        SetConsoleTitleW(L"SS_Enhance");

        // 대체 화면 버퍼를 사용해 종료 후 사용자의 기존 콘솔 내용을 그대로 복원한다.
        WriteControlSequence(L"\x1b[?1049h\x1b[2J\x1b[?25l");
    }

    ConsoleSession::~ConsoleSession() noexcept
    {
        // 커서와 화면 버퍼를 먼저 복원한 뒤 저장해 둔 콘솔 모드로 되돌린다.
        WriteControlSequence(L"\x1b[0m\x1b[?25h\x1b[?1049l");

        if (hasOutputMode_)
        {
            SetConsoleMode(outputHandle_, oldOutputMode_);
        }
        if (hasInputMode_)
        {
            SetConsoleMode(inputHandle_, oldInputMode_);
        }
        if (hasOldTitle_)
        {
            SetConsoleTitleW(oldTitle_.c_str());
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
