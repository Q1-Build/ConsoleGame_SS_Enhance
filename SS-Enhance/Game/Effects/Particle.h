#pragma once

#include "Rendering/Color.h"

namespace ss
{
    /// 화면 효과 파티클 한 개의 물리 상태와 시각 정보를 보관한다.
    struct Particle
    {
        float x = 0.0f;
        float y = 0.0f;
        float velocityX = 0.0f;
        float velocityY = 0.0f;
        float life = 0.0f;
        float maxLife = 0.0f;
        wchar_t glyph = L'*';
        Color color = Color::Yellow;
    };
}
