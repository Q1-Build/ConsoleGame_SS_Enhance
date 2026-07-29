#pragma once

namespace ss
{
    /// 플레이어 검의 강화 단계와 등급을 관리하는 도메인 객체다.
    class Sword final
    {
    public:
        [[nodiscard]] int GetLevel() const noexcept;
        [[nodiscard]] int GetTier() const noexcept;

        void Enhance(int levelCount = 1);
        void LoseLevel();

    private:
        // 강화 단계는 항상 0 이상을 유지한다.
        int level_ = 0;
    };
}
