#pragma once

namespace ss
{
    /// 난수 생성 방식과 게임 로직 사이의 의존성을 분리하는 계약이다.
    class IRandomProvider
    {
    public:
        virtual ~IRandomProvider() = default;

        /// 지정한 범위 안의 실수 난수를 반환한다.
        [[nodiscard]] virtual float NextFloat(float minValue, float maxValue) = 0;
    };
}
