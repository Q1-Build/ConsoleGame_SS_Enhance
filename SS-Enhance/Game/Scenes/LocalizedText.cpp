#include "Game/Scenes/LocalizedText.h"

#include <algorithm>
#include <array>

namespace ss
{
    std::wstring_view LocalizedText::GetSwordName(
        Language language,
        int swordTier) noexcept
    {
        // 이름은 화면 표현이므로 검의 강화 규칙과 분리하고, 잘못된 등급도 안전하게 보정한다.
        static constexpr std::array<std::wstring_view, 7> kKoreanNames =
        {
            L"이름 없는 철검",
            L"잿불의 칼날",
            L"진홍의 맹세",
            L"폭풍의 송곳니",
            L"달빛의 진혼곡",
            L"공허 절단자",
            L"별을 삼키는 자"
        };
        static constexpr std::array<std::wstring_view, 7> kEnglishNames =
        {
            L"Nameless Iron",
            L"Ember Edge",
            L"Crimson Oath",
            L"Storm Fang",
            L"Moonlit Requiem",
            L"Void Divider",
            L"Star Eater"
        };

        const int safeTier = std::clamp(
            swordTier,
            0,
            static_cast<int>(kKoreanNames.size()) - 1);
        return language == Language::Korean
            ? kKoreanNames[static_cast<std::size_t>(safeTier)]
            : kEnglishNames[static_cast<std::size_t>(safeTier)];
    }
}
