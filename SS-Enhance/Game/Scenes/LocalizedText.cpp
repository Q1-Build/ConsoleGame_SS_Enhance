#include "Game/Scenes/LocalizedText.h"

#include "Game/Domain/Difficulty.h"
#include "Game/Domain/BossBattle.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <sstream>

namespace ss
{
    namespace
    {
        /// 한 보스의 한국어·영어 화면 문구를 함께 보관하는 표현 전용 데이터다.
        struct BossTextSet
        {
            std::wstring_view koreanName;
            std::wstring_view englishName;
            std::wstring_view koreanPattern;
            std::wstring_view englishPattern;
            std::wstring_view koreanIntroduction;
            std::wstring_view englishIntroduction;
            std::wstring_view koreanDefeat;
            std::wstring_view englishDefeat;
        };

        constexpr std::array<BossTextSet, 3> kBossTexts =
        {{
            {
                L"잿불의 문지기",
                L"EMBER WARDEN",
                L"긴 차징 뒤 이어지는 강한 일격",
                L"LONG CHARGE, CRUSHING BLOW",
                L"갑옷 틈의 잿불이 거대한 불길로 번집니다.",
                L"EMBERS BETWEEN ITS PLATES ERUPT INTO FLAME.",
                L"무거운 갑옷이 잿가루가 되어 무너집니다.",
                L"THE HEAVY ARMOR COLLAPSES INTO ASH."
            },
            {
                L"폭풍의 파수꾼",
                L"STORM SENTINEL",
                L"점점 빨라지는 삼연격",
                L"AN ACCELERATING TRIPLE COMBO",
                L"세 번의 천둥이 서로 다른 박자로 울립니다.",
                L"THREE THUNDERS ANSWER IN DIFFERENT RHYTHMS.",
                L"마지막 번개가 끊어지며 폭풍이 잠잠해집니다.",
                L"THE FINAL BOLT BREAKS, AND THE STORM FALLS SILENT."
            },
            {
                L"기억을 삼키는 자",
                L"MEMORY DEVOURER",
                L"보랏빛 잔상과 뒤틀린 예고",
                L"VIOLET FEINTS AND DISTORTED WARNINGS",
                L"진짜 공격과 보랏빛 잔상이 기억 속에서 겹칩니다.",
                L"TRUE STRIKES AND VIOLET AFTERIMAGES OVERLAP.",
                L"뒤엉킨 잔상 속에서 마지막 기억이 돌아옵니다.",
                L"THE FINAL MEMORY RETURNS FROM THE FRACTURED ECHOES."
            }
        }};

        [[nodiscard]] constexpr std::size_t GetBossTextIndex(
            BossType bossType) noexcept
        {
            switch (bossType)
            {
            case BossType::EmberWarden:
                return 0;
            case BossType::StormSentinel:
                return 1;
            case BossType::MemoryDevourer:
                return 2;
            }
            return 0;
        }
    }

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
        // enum 값과 화면 이름의 대응은 설정 장면 밖에서도 재사용할 수 있도록 한곳에 둔다.
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
                    << Select(language, L"% | ", L"% | ")
                    << static_cast<int>(tuning.forgeDurationSeconds)
                    << Select(language, L"초 | 하락 +", L"s | LOSS +")
                    << tuning.failurePenaltyStartLevel
                    << Select(language, L"부터 | ", L"+ | ");

        // 수치는 Domain 튜닝에서 가져오고 체감 설명만 난이도별 번역으로 덧붙인다.
        switch (difficulty)
        {
        case Difficulty::Easy:
            description << Select(
                language, L"느린 냉각", L"SLOW COOLING");
            break;
        case Difficulty::Normal:
            description << Select(
                language, L"기본 냉각", L"STANDARD COOLING");
            break;
        case Difficulty::Hard:
            description << Select(
                language, L"빠른 냉각", L"FAST COOLING");
            break;
        }
        return description.str();
    }

    std::wstring_view LocalizedText::GetBossName(
        Language language,
        BossType bossType) noexcept
    {
        // 도메인에는 표시 문자열을 넣지 않고 보스 식별 값만 표현 표의 키로 사용한다.
        const BossTextSet& text = kBossTexts[GetBossTextIndex(bossType)];
        return Select(language, text.koreanName, text.englishName);
    }

    std::wstring_view LocalizedText::GetBossPatternDescription(
        Language language,
        BossType bossType) noexcept
    {
        // 상세 수치를 반복하지 않고 전투 전에 알아야 할 패턴의 차이만 짧게 전달한다.
        const BossTextSet& text = kBossTexts[GetBossTextIndex(bossType)];
        return Select(language, text.koreanPattern, text.englishPattern);
    }

    std::wstring_view LocalizedText::GetBossIntroduction(
        Language language,
        BossType bossType) noexcept
    {
        const BossTextSet& text = kBossTexts[GetBossTextIndex(bossType)];
        return Select(
            language,
            text.koreanIntroduction,
            text.englishIntroduction);
    }

    std::wstring_view LocalizedText::GetBossDefeatText(
        Language language,
        BossType bossType) noexcept
    {
        const BossTextSet& text = kBossTexts[GetBossTextIndex(bossType)];
        return Select(language, text.koreanDefeat, text.englishDefeat);
    }
}
