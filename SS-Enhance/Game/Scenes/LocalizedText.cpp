#include "Game/Scenes/LocalizedText.h"

#include "Game/Domain/Difficulty.h"
#include "Game/Domain/BossBattle.h"

#include <algorithm>
#include <array>
#include <sstream>

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

    std::wstring_view LocalizedText::GetDifficultyName(
        Language language,
        Difficulty difficulty) noexcept
    {
        switch (difficulty)
        {
        case Difficulty::Easy:
            return Select(language, L"쉬움", L"EASY");
        case Difficulty::Normal:
            return Select(language, L"보통", L"NORMAL");
        case Difficulty::Hard:
            return Select(language, L"어려움", L"HARD");
        }
        return Select(language, L"보통", L"NORMAL");
    }

    std::wstring LocalizedText::GetDifficultyDescription(
        Language language,
        Difficulty difficulty)
    {
        const DifficultyTuning tuning = GetDifficultyTuning(difficulty);
        std::wstringstream description;
        description << Select(language, L"비용 ", L"COST ")
                    << tuning.forgeCostPercent
                    << Select(language, L"% | 제한 ", L"% | ")
                    << static_cast<int>(tuning.forgeDurationSeconds)
                    << Select(language, L"초 | ", L" sec | ");

        // 수치는 Domain 튜닝에서 가져오고 체감 설명만 난이도별 번역으로 덧붙인다.
        switch (difficulty)
        {
        case Difficulty::Easy:
            description << Select(
                language, L"온도가 천천히 내려갑니다.", L"Heat falls slowly.");
            break;
        case Difficulty::Normal:
            description << Select(
                language, L"기본 온도 속도", L"Standard heat speed");
            break;
        case Difficulty::Hard:
            description << Select(
                language, L"온도가 빠르게 내려갑니다.", L"Heat falls quickly.");
            break;
        }
        return description.str();
    }

    std::wstring_view LocalizedText::GetBossName(
        Language language,
        BossType bossType) noexcept
    {
        switch (bossType)
        {
        case BossType::EmberWarden:
            return Select(language, L"잿불의 문지기", L"EMBER WARDEN");
        case BossType::StormSentinel:
            return Select(language, L"폭풍의 파수꾼", L"STORM SENTINEL");
        case BossType::MemoryDevourer:
            return Select(language, L"기억을 삼키는 자", L"MEMORY DEVOURER");
        }
        return {};
    }
}
