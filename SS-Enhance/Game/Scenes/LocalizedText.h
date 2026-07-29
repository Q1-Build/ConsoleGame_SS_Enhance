#pragma once

#include "Core/Language.h"

#include <string_view>

namespace ss
{
    /// 현재 언어에 맞는 화면 문구와 검 이름을 선택한다.
    class LocalizedText final
    {
    public:
        /// 두 번역 중 현재 언어에 맞는 문자열을 반환한다.
        [[nodiscard]] static constexpr std::wstring_view Select(
            Language language,
            std::wstring_view korean,
            std::wstring_view english) noexcept
        {
            return language == Language::Korean ? korean : english;
        }

        /// 검 등급에 맞는 현지화된 이름을 반환한다.
        [[nodiscard]] static std::wstring_view GetSwordName(
            Language language,
            int swordTier) noexcept;
    };
}
