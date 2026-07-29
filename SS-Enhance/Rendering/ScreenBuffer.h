#pragma once

#include "Rendering/Cell.h"
#include "Rendering/IScreen.h"

#include <string>
#include <vector>

namespace ss
{
    /// 장면이 그린 문자를 메모리에 보관하고 하나의 ANSI 프레임으로 조립한다.
    class ScreenBuffer final : public IScreen
    {
    public:
        /// 공통 화면 크기만큼 빈 셀 버퍼를 생성한다.
        ScreenBuffer();

        /// 전체 셀을 지정한 문자와 색상으로 초기화하고 전각 점유 정보를 제거한다.
        void Clear(wchar_t glyph = L' ', Color color = Color::White) override;

        /// 한 문자를 기록하며 전각 문자는 연속된 두 화면 칸을 점유한다.
        void Put(int x, int y, wchar_t glyph, Color color = Color::White) override;

        /// 지정한 좌표부터 실제 콘솔 표시 폭을 반영해 문자열을 기록한다.
        void Text(int x, int y, std::wstring_view text, Color color = Color::White) override;

        /// 한글과 CJK 전각 문자를 두 칸으로 계산한 문자열 너비를 반환한다.
        [[nodiscard]] int MeasureText(std::wstring_view text) const noexcept override;

        /// 실제 표시 폭을 기준으로 문자열 끝을 지정한 오른쪽 좌표에 맞춘다.
        void RightText(
            int right,
            int y,
            std::wstring_view text,
            Color color = Color::White) override;

        /// 실제 표시 폭을 기준으로 지정한 좌우 영역 안에 문자열을 가운데 정렬한다.
        void CenterTextIn(
            int left,
            int right,
            int y,
            std::wstring_view text,
            Color color = Color::White) override;

        /// 실제 표시 폭을 기준으로 전체 화면 가운데에 문자열을 정렬한다.
        void CenterText(int y, std::wstring_view text, Color color = Color::White) override;

        /// 두 좌표를 포함하는 직선을 셀 버퍼에 그린다.
        void Line(int x1, int y1, int x2, int y2, wchar_t glyph, Color color) override;

        /// 지정한 좌표 경계를 포함하는 사각형 테두리를 셀 버퍼에 그린다.
        void Box(int left, int top, int right, int bottom, Color color) override;

        /// 현재 버퍼를 커서 이동과 색상 코드를 포함한 출력 문자열로 만든다.
        [[nodiscard]] std::wstring BuildAnsiFrame() const;

    private:
        // 좌표 검사와 문자 폭 계산은 모든 그리기 진입점에서 같은 셀 규칙을 공유한다.
        [[nodiscard]] bool IsInside(int x, int y) const noexcept;
        [[nodiscard]] static int GetDisplayWidth(wchar_t glyph) noexcept;
        [[nodiscard]] static int GetDisplayWidth(std::wstring_view text) noexcept;

        // 전각 문자의 시작 칸과 연속 칸 중 어느 쪽을 덮어써도 기존 점유를 함께 제거한다.
        void ClearOccupiedCell(int x, int y);

        // 화면 좌표를 행 우선 순서로 저장하는 고정 크기 셀 버퍼다.
        std::vector<Cell> cells_;
    };
}
