#pragma once

#define NOMINMAX
#include <Windows.h>

namespace ss
{
    /// 프로그램 실행 동안 Windows 콘솔 모드를 설정하고 종료 시 원래 상태로 복원한다.
    class ConsoleSession final
    {
    public:
        ConsoleSession();
        ~ConsoleSession() noexcept;

        ConsoleSession(const ConsoleSession&) = delete;
        ConsoleSession& operator=(const ConsoleSession&) = delete;
        ConsoleSession(ConsoleSession&&) = delete;
        ConsoleSession& operator=(ConsoleSession&&) = delete;

    private:
        static void WriteControlSequence(const wchar_t* text) noexcept;

        // 복원해야 하는 콘솔 핸들과 원래 입출력 모드다.
        HANDLE outputHandle_ = INVALID_HANDLE_VALUE;
        HANDLE inputHandle_ = INVALID_HANDLE_VALUE;
        DWORD oldOutputMode_ = 0;
        DWORD oldInputMode_ = 0;
        bool hasOutputMode_ = false;
        bool hasInputMode_ = false;
    };
}
