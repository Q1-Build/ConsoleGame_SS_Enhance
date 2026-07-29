#include "Game/Domain/Sword.h"

#include <algorithm>
#include <array>
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

    std::wstring_view Sword::GetName() const noexcept
    {
        // 두 강화 단계마다 이름 등급이 바뀌며 마지막 이름 이후에는 최고 등급을 유지한다.
        static constexpr std::array<std::wstring_view, 7> kNames =
        {
            L"Nameless Iron",
            L"Ember Edge",
            L"Crimson Oath",
            L"Storm Fang",
            L"Moonlit Requiem",
            L"Void Divider",
            L"Star Eater"
        };
        return kNames[GetTier()];
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
