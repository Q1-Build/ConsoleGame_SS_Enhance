#pragma once

#include "Core/Language.h"

#include <deque>
#include <string>

namespace ss
{
    class IInput;
    class IScreen;

    /// 입력한 메시지를 모든 장면 위에 유지하고 우측 하단 채팅 박스로 표시한다.
    class PresentationChatOverlay final
    {
    public:
        /// 채팅 열기·편집·등록 입력을 처리하고 이번 프레임의 게임 입력 차단 여부를 반환한다.
        [[nodiscard]] bool Update(const IInput& input, const IScreen& screen);

        /// 최근 메시지와 현재 입력문을 우측 하단 채팅 영역에 그린다.
        void Draw(IScreen& screen, Language language) const;

    private:
        void SubmitCurrentInput(const IScreen& screen);
        [[nodiscard]] std::wstring GetVisibleInput(const IScreen& screen) const;

        // 확정된 줄은 박스에 표시 가능한 수만 보관하고 편집문은 등록 전까지 별도로 유지한다.
        std::deque<std::wstring> lines_;
        std::wstring currentInput_;
        bool isEditing_ = false;
    };
}
