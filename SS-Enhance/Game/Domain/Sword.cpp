#include "Game/Domain/Sword.h"

#include <algorithm>
#include <cassert>

namespace ss
{
    int Sword::GetLevel() const noexcept
    {
        return level_;
    }

    int Sword::GetTier() const noexcept
    {
        return std::min(level_ / 2, 6);
    }

    void Sword::Enhance(int levelCount)
    {
        assert(levelCount > 0);
        level_ += levelCount;
    }

    void Sword::LoseLevel()
    {
        // 실패가 누적되어도 도메인 불변 조건인 최소 0단계는 깨지지 않는다.
        level_ = std::max(0, level_ - 1);
    }
}
