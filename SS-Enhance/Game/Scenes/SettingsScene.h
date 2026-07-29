#pragma once

#include "Game/Scenes/IScene.h"

namespace ss
{
    struct SceneContext;

    /// 본게임 진입 전에 표시 언어와 게임 난이도를 선택하는 장면이다.
    class SettingsScene final : public IScene
    {
    public:
        /// 애플리케이션이 소유한 공유 설정을 비소유 참조로 연결한다.
        explicit SettingsScene(SceneContext& context);

        void OnEnter() override;
        [[nodiscard]] SceneTransition Update(float deltaSeconds) override;
        void Render(IScreen& screen) const override;
        void OnExit() override;

    private:
        /// 설정 화면에서 현재 좌우 입력을 받을 항목이다.
        enum class SettingItem
        {
            Language,
            Difficulty
        };

        /// 현재 항목의 이전 값을 선택하며 끝에서는 마지막 값으로 순환한다.
        void SelectPreviousValue();

        /// 현재 항목의 다음 값을 선택하며 끝에서는 첫 값으로 순환한다.
        void SelectNextValue();

        // 공유 설정은 SceneContext에 저장하고 현재 메뉴 위치만 장면이 소유한다.
        SceneContext& context_;
        SettingItem selectedItem_ = SettingItem::Language;
    };
}
