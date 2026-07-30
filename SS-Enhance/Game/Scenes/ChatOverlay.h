#pragma once

#include "Core/Language.h"

#include <deque>
#include <string>

namespace ss
{
    class IInput;
    class IScreen;

    /// 입력한 메시지를 모든 장면 위에 유지하고 우측 하단 채팅 박스로 표시한다.
    class ChatOverlay final
    {
    public:
        /// 채팅 열기·편집·등록 입력을 처리하고 이번 프레임의 게임 입력 차단 여부를 반환한다.
        [[nodiscard]] bool Update(const IInput& input, const IScreen& screen);

        /// 최근 메시지와 현재 입력문을 우측 하단 채팅 영역에 그린다.
        void Draw(IScreen& screen, Language language) const;

    private:
        void SubmitCurrentInput(const IScreen& screen);
        [[nodiscard]] std::wstring GetVisibleInput(const IScreen& screen) const;

        // 확정 줄과 편집문을 분리하고 채팅 열기 키의 지연 문자 소비 여부를 함께 추적한다.
        std::deque<std::wstring> lines_;
        std::wstring currentInput_;
        bool isEditing_ = false;
        bool isWaitingForOpeningCharacter_ = false;
    };
}
