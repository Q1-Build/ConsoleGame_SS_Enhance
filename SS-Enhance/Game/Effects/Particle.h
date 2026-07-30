#pragma once

#include "Rendering/Color.h"

namespace ss
{
    /// 화면 효과 파티클 한 개의 물리 상태와 시각 정보를 보관한다.
    struct Particle
    {
        // 위치와 초당 이동량은 부드러운 프레임 보간을 위해 화면 셀 좌표의 실수값으로 저장한다.
        float x = 0.0f;
        float y = 0.0f;
        float velocityX = 0.0f;
        float velocityY = 0.0f;

        // 현재 남은 수명과 최초 수명은 초 단위이며 투명도 대신 색상 단계 계산에도 사용한다.
        float life = 0.0f;
        float maxLife = 0.0f;

        // 화면에 출력할 단일 문자와 ANSI 색상이다.
        wchar_t glyph = L'*';
        Color color = Color::Yellow;
    };
}
