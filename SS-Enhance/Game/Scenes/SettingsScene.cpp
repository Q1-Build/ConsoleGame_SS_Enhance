#include "Game/Scenes/SettingsScene.h"

#include "Game/Domain/Difficulty.h"
#include "Game/Effects/ParticleSystem.h"
#include "Game/Scenes/GameHudRenderer.h"
#include "Game/Scenes/LocalizedText.h"
#include "Game/Scenes/SceneContext.h"
#include "Platform/IAudio.h"
#include "Platform/IInput.h"
#include "Rendering/IScreen.h"

#include <algorithm>
#include <string>

namespace ss
{
    namespace
    {
        constexpr int kVolumeStepPercent = 10;
        constexpr int kMaximumVolumePercent = 100;
        constexpr int kVolumeSegmentCount =
            kMaximumVolumePercent / kVolumeStepPercent;
    }

    SettingsScene::SettingsScene(SceneContext& context)
        : context_(context)
    {
    }

    void SettingsScene::OnEnter()
    {
        selectedItem_ = SettingItem::Language;
        ApplyMasterVolume();
        context_.audio.PlayMusic(MusicTrack::Title);
    }

    SceneTransition SettingsScene::Update(float deltaSeconds)
    {
        constexpr float kAmbientEmbersPerSecond = 5.0f;
        context_.particles.EmitAmbientEmbers(
            deltaSeconds,
            kAmbientEmbersPerSecond);

        if (context_.input.WasPressed(InputKey::Escape))
        {
            context_.audio.PlaySound(SoundEffect::MenuBack);
            return SceneTransition::To(SceneType::Title);
        }

        // 위아래 입력은 세 항목을 순서대로 순환하고 좌우 입력은 선택한 값만 변경한다.
        if (context_.input.WasPressed(InputKey::Up) ||
            context_.input.WasPressed(InputKey::W))
        {
            SelectPreviousItem();
            context_.audio.PlaySound(SoundEffect::MenuMove);
        }
        if (context_.input.WasPressed(InputKey::Down) ||
            context_.input.WasPressed(InputKey::S))
        {
            SelectNextItem();
            context_.audio.PlaySound(SoundEffect::MenuMove);
        }
        if (context_.input.WasPressed(InputKey::Left) ||
            context_.input.WasPressed(InputKey::A))
        {
            SelectPreviousValue();
            context_.audio.PlaySound(SoundEffect::MenuMove);
        }
        if (context_.input.WasPressed(InputKey::Right) ||
            context_.input.WasPressed(InputKey::D))
        {
            SelectNextValue();
            context_.audio.PlaySound(SoundEffect::MenuMove);
        }

        if (context_.input.WasPressed(InputKey::Enter) ||
            context_.input.WasPressed(InputKey::Space))
        {
            context_.audio.PlaySound(SoundEffect::MenuConfirm);
            return SceneTransition::To(SceneType::Forge);
        }
        return SceneTransition::None();
    }

    void SettingsScene::Render(IScreen& screen) const
    {
        context_.hudRenderer.DrawBackdrop(screen, context_.worldTimeSeconds);
        screen.Box(18, 5, 85, 29, Color::BrightBlack);

        // 언어를 바꾸는 즉시 설정 화면 전체 문구도 선택한 언어로 갱신한다.
        screen.CenterText(
            7,
            LocalizedText::Select(
                context_.language,
                L"게임 설정",
                L"GAME SETTINGS"),
            Color::BrightRed);
        screen.CenterText(
            9,
            LocalizedText::Select(
                context_.language,
                L"위아래로 항목을 고르고 좌우로 변경하세요.",
                L"Choose with up/down and change with left/right."),
            Color::BrightBlack);

        const Color languageColor = selectedItem_ == SettingItem::Language
            ? Color::BrightYellow
            : Color::White;
        const Color volumeColor = selectedItem_ == SettingItem::MasterVolume
            ? Color::BrightYellow
            : Color::White;
        const Color difficultyColor = selectedItem_ == SettingItem::Difficulty
            ? Color::BrightYellow
            : Color::White;

        // 선택 화살표와 강조색을 함께 사용해 흑백 콘솔에서도 현재 항목을 구분한다.
        screen.Text(
            27,
            13,
            selectedItem_ == SettingItem::Language ? L"▶" : L" ",
            languageColor);
        screen.Text(
            31,
            13,
            LocalizedText::Select(context_.language, L"언어", L"LANGUAGE"),
            languageColor);
        screen.CenterTextIn(
            48,
            78,
            13,
            context_.language == Language::Korean ? L"◀  한국어  ▶" : L"◀  ENGLISH  ▶",
            languageColor);

        screen.Text(
            27,
            16,
            selectedItem_ == SettingItem::MasterVolume ? L"▶" : L" ",
            volumeColor);
        screen.Text(
            31,
            16,
            LocalizedText::Select(
                context_.language,
                L"마스터 볼륨",
                L"MASTER VOLUME"),
            volumeColor);

        std::wstring volumeSlider = L"◀  [";
        const int filledSegments =
            context_.masterVolumePercent / kVolumeStepPercent;
        for (int index = 0; index < kVolumeSegmentCount; ++index)
        {
            volumeSlider += index < filledSegments ? L'█' : L'░';
        }
        volumeSlider += L"] ";
        volumeSlider += std::to_wstring(context_.masterVolumePercent);
        volumeSlider += L"%  ▶";
        screen.CenterTextIn(
            48,
            78,
            16,
            volumeSlider,
            volumeColor);

        screen.Text(
            27,
            20,
            selectedItem_ == SettingItem::Difficulty ? L"▶" : L" ",
            difficultyColor);
        screen.Text(
            31,
            20,
            LocalizedText::Select(context_.language, L"난이도", L"DIFFICULTY"),
            difficultyColor);
        // 화살표와 값을 한 문자열로 묶어 난이도 이름 길이가 달라도 중심을 유지한다.
        std::wstring difficultyValue = L"◀  ";
        difficultyValue += LocalizedText::GetDifficultyName(
            context_.language,
            context_.difficulty);
        difficultyValue += L"  ▶";
        screen.CenterTextIn(
            48,
            78,
            20,
            difficultyValue,
            difficultyColor);

        screen.CenterTextIn(
            19,
            84,
            23,
            LocalizedText::GetDifficultyDescription(
                context_.language,
                context_.difficulty),
            Color::BrightCyan);
        screen.CenterTextIn(
            19,
            84,
            26,
            LocalizedText::Select(
                context_.language,
                L"[ ENTER ] 설정 완료 | 게임 시작",
                L"[ ENTER ] APPLY | START GAME"),
            Color::BrightYellow);
        screen.CenterTextIn(
            19,
            84,
            28,
            LocalizedText::Select(
                context_.language,
                L"[ ESC ] 타이틀로",
                L"[ ESC ] BACK TO TITLE"),
            Color::BrightBlack);
    }

    void SettingsScene::OnExit()
    {
    }

    void SettingsScene::SelectPreviousItem()
    {
        switch (selectedItem_)
        {
        case SettingItem::Language:
            selectedItem_ = SettingItem::Difficulty;
            break;
        case SettingItem::MasterVolume:
            selectedItem_ = SettingItem::Language;
            break;
        case SettingItem::Difficulty:
            selectedItem_ = SettingItem::MasterVolume;
            break;
        }
    }

    void SettingsScene::SelectNextItem()
    {
        switch (selectedItem_)
        {
        case SettingItem::Language:
            selectedItem_ = SettingItem::MasterVolume;
            break;
        case SettingItem::MasterVolume:
            selectedItem_ = SettingItem::Difficulty;
            break;
        case SettingItem::Difficulty:
            selectedItem_ = SettingItem::Language;
            break;
        }
    }

    void SettingsScene::SelectPreviousValue()
    {
        if (selectedItem_ == SettingItem::Language)
        {
            context_.language = context_.language == Language::Korean
                ? Language::English
                : Language::Korean;
            return;
        }

        if (selectedItem_ == SettingItem::MasterVolume)
        {
            context_.masterVolumePercent = std::max(
                0,
                context_.masterVolumePercent - kVolumeStepPercent);
            ApplyMasterVolume();
            return;
        }

        // 세 난이도는 양 끝에서 순환해 좌우 어느 방향으로도 빠르게 선택할 수 있다.
        switch (context_.difficulty)
        {
        case Difficulty::Easy:
            context_.difficulty = Difficulty::Hard;
            break;
        case Difficulty::Normal:
            context_.difficulty = Difficulty::Easy;
            break;
        case Difficulty::Hard:
            context_.difficulty = Difficulty::Normal;
            break;
        }
    }

    void SettingsScene::SelectNextValue()
    {
        if (selectedItem_ == SettingItem::Language)
        {
            context_.language = context_.language == Language::Korean
                ? Language::English
                : Language::Korean;
            return;
        }

        if (selectedItem_ == SettingItem::MasterVolume)
        {
            context_.masterVolumePercent = std::min(
                kMaximumVolumePercent,
                context_.masterVolumePercent + kVolumeStepPercent);
            ApplyMasterVolume();
            return;
        }

        switch (context_.difficulty)
        {
        case Difficulty::Easy:
            context_.difficulty = Difficulty::Normal;
            break;
        case Difficulty::Normal:
            context_.difficulty = Difficulty::Hard;
            break;
        case Difficulty::Hard:
            context_.difficulty = Difficulty::Easy;
            break;
        }
    }

    void SettingsScene::ApplyMasterVolume()
    {
        context_.audio.SetMasterVolume(
            static_cast<float>(context_.masterVolumePercent) /
            static_cast<float>(kMaximumVolumePercent));
    }
}
