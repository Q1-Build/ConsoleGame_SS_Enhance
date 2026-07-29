#pragma once

namespace ss
{
    /// 애플리케이션이 생성할 수 있는 장면 종류다.
    enum class SceneType
    {
        Title,
        Settings,
        Forge,
        Forging,
        Result,
        Exit
    };

    /// 장면이 직접 다른 장면을 생성하지 않고 전환 의도만 반환하는 값 타입이다.
    struct SceneTransition
    {
        bool isRequested = false;
        SceneType target = SceneType::Title;

        [[nodiscard]] static constexpr SceneTransition None() noexcept
        {
            return {};
        }

        [[nodiscard]] static constexpr SceneTransition To(SceneType targetScene) noexcept
        {
            return {true, targetScene};
        }
    };
}
