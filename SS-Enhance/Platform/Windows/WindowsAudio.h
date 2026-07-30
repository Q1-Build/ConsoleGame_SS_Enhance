#pragma once

#include "Platform/IAudio.h"

#include <filesystem>
#include <optional>

namespace ss
{
    /// WinMM을 사용해 배경음악과 효과음을 서로 독립된 재생 경로로 출력한다.
    /// 음원 누락이나 재생 실패는 게임 진행을 중단하지 않고 무음으로 처리한다.
    class WindowsAudio final : public IAudio
    {
    public:
        /// 실행 파일 위치를 기준으로 배포 음원 디렉터리를 결정한다.
        WindowsAudio();

        /// 진행 중인 BGM과 효과음을 정지하고 모든 MCI 별칭을 닫는다.
        ~WindowsAudio() override;

        WindowsAudio(const WindowsAudio&) = delete;
        WindowsAudio& operator=(const WindowsAudio&) = delete;
        WindowsAudio(WindowsAudio&&) = delete;
        WindowsAudio& operator=(WindowsAudio&&) = delete;

        void PlayMusic(MusicTrack track) override;
        void StopMusic() override;
        void PlaySound(SoundEffect effect) override;
        void StopSounds() override;
        void SetMasterVolume(float volume) override;

    private:
        [[nodiscard]] static std::filesystem::path GetExecutableDirectory();
        [[nodiscard]] std::filesystem::path GetMusicPath(MusicTrack track) const;
        [[nodiscard]] std::filesystem::path GetEffectPath(SoundEffect effect) const;
        [[nodiscard]] bool ApplyVolume(const wchar_t* alias) const;

        // 배포된 실행 파일을 기준으로 Assets/Audio를 찾고 현재 MCI 별칭의 수명을 추적한다.
        std::filesystem::path audioDirectory_;
        std::optional<MusicTrack> currentMusic_;
        bool hasActiveSound_ = false;
        float masterVolume_ = 1.0f;
    };
}
