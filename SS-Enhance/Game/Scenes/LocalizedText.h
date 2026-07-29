#pragma once

#include "Core/Language.h"

#include <string>
#include <string_view>

namespace ss
{
    enum class Difficulty;

    /// 현재 언어에 맞는 화면 문구와 검 이름을 선택한다.
    class LocalizedText final
    {
    public:
        /// 두 번역 중 현재 언어에 맞는 문자열을 반환한다.
        /// 반환된 뷰는 호출자가 전달한 문자열보다 오래 보관하지 않는다.
        /// @param language 선택할 표시 언어
        /// @param korean 한국어로 표시할 문자열
        /// @param english 영어로 표시할 문자열
        [[nodiscard]] static constexpr std::wstring_view Select(
            Language language,
            std::wstring_view korean,
            std::wstring_view english) noexcept
        {
            return language == Language::Korean ? korean : english;
        }

        /// 검 등급에 맞는 현지화된 이름을 반환한다.
        /// @param language 선택할 표시 언어
        /// @param swordTier 검의 이름 등급이며 유효 범위를 벗어나면 0~6으로 보정한다.
        /// @return 프로그램 수명 동안 유효한 현지화 문자열 뷰
        [[nodiscard]] static std::wstring_view GetSwordName(
            Language language,
            int swordTier) noexcept;

        /// 선택한 게임 난이도의 현지화된 이름을 반환한다.
        [[nodiscard]] static std::wstring_view GetDifficultyName(
            Language language,
            Difficulty difficulty) noexcept;

        /// 선택한 게임 난이도의 비용, 제한 시간과 온도 특성을 설명한다.
        [[nodiscard]] static std::wstring GetDifficultyDescription(
            Language language,
            Difficulty difficulty);
    };
}
