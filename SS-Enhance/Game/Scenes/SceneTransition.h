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
        Battle,
        Ending,
        Exit
    };

    /// 장면이 직접 다른 장면을 생성하지 않고 전환 의도만 반환하는 값 타입이다.
    struct SceneTransition
    {
        // 요청이 없을 때 target은 읽지 않으며 요청이 있을 때만 다음 장면을 나타낸다.
        bool isRequested = false;
        SceneType target = SceneType::Title;

        /// 현재 장면을 유지하는 전환 결과를 만든다.
        [[nodiscard]] static constexpr SceneTransition None() noexcept
        {
            return {};
        }

        /// 지정한 장면으로 이동하도록 애플리케이션에 요청하는 결과를 만든다.
        [[nodiscard]] static constexpr SceneTransition To(SceneType targetScene) noexcept
        {
            return {true, targetScene};
        }
    };
}
