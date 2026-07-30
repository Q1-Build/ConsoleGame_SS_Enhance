#pragma once

namespace ss
{
    /// 플레이어 검의 강화 단계와 등급을 관리하는 도메인 객체다.
    class Sword final
    {
    public:
        /// 현재 강화 단계와 단계에서 파생된 4단위 등급 인덱스를 반환한다.
        [[nodiscard]] int GetLevel() const noexcept;
        [[nodiscard]] int GetTier() const noexcept;

        /// 양수 단계 수만큼 강화하며 호출자가 진행 구간 상한을 먼저 검증해야 한다.
        void Enhance(int levelCount = 1);

        /// 현재 단계를 한 단계 낮추되 0 아래로 내려가지 않게 한다.
        void LoseLevel();

    private:
        // 강화 단계는 항상 0 이상을 유지한다.
        int level_ = 0;
    };
}
