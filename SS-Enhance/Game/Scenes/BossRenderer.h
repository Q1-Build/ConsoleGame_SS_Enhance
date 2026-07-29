#pragma once

#include "Game/Domain/BossBattle.h"
#include "Rendering/Color.h"

namespace ss
{
    class IScreen;

    /// 보스 종류별 아스키 형상, 대표 색상과 등장·격파 상태를 화면에 표현한다.
    class BossRenderer final
    {
    public:
        /// 등장과 격파 진행도는 0~1 범위이며 장면 상태를 변경하지 않는다.
        void Draw(
            IScreen& screen,
            BossType bossType,
            float introductionProgress,
            float defeatProgress,
            int shakeX,
            float worldTimeSeconds) const;

        [[nodiscard]] Color GetPrimaryColor(BossType bossType) const noexcept;
    };
}
