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
        level_ = std::max(0, level_ - 1);
    }
}
