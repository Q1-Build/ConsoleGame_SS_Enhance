#include "Platform/Windows/WindowsAudio.h"

#define NOMINMAX
#include <Windows.h>
#include <mmsystem.h>

#ifdef PlaySound
#undef PlaySound
#endif

#include <array>
#include <cassert>
#include <system_error>
#include <string>

#pragma comment(lib, "winmm.lib")

namespace ss
{
    namespace
    {
        constexpr wchar_t kMusicAlias[] = L"ss_enhance_bgm";
    }

    WindowsAudio::WindowsAudio()
        : audioDirectory_(GetExecutableDirectory() / L"Assets" / L"Audio")
    {
    }

    WindowsAudio::~WindowsAudio()
    {
        StopMusic();
        // 비동기 효과음 콜백이 객체 수명 뒤까지 남지 않도록 프로세스 종료 전에 정지한다.
        StopSounds();
    }

    void WindowsAudio::PlayMusic(MusicTrack track)
    {
        if (currentMusic_.has_value() && *currentMusic_ == track)
        {
            return;
        }

        StopMusic();
        const std::filesystem::path path = GetMusicPath(track);
        std::error_code fileError;
        if (!std::filesystem::exists(path, fileError) || fileError)
        {
            return;
        }

        // mpegvideo 드라이버는 WAV 반복 명령을 지원해 효과음용 PlaySound와 재생 경로를 분리한다.
        const std::wstring openCommand =
            L"open \"" + path.wstring() + L"\" type mpegvideo alias " + kMusicAlias;
        if (mciSendStringW(openCommand.c_str(), nullptr, 0, nullptr) != 0)
        {
            return;
        }

        const std::wstring playCommand =
            std::wstring(L"play ") + kMusicAlias + L" repeat";
        if (mciSendStringW(playCommand.c_str(), nullptr, 0, nullptr) != 0)
        {
            const std::wstring closeCommand =
                std::wstring(L"close ") + kMusicAlias;
            mciSendStringW(closeCommand.c_str(), nullptr, 0, nullptr);
            return;
        }
        currentMusic_ = track;
    }

    void WindowsAudio::StopMusic()
    {
        if (!currentMusic_.has_value())
        {
            return;
        }

        const std::wstring stopCommand =
            std::wstring(L"stop ") + kMusicAlias;
        const std::wstring closeCommand =
            std::wstring(L"close ") + kMusicAlias;
        mciSendStringW(stopCommand.c_str(), nullptr, 0, nullptr);
        mciSendStringW(closeCommand.c_str(), nullptr, 0, nullptr);
        currentMusic_.reset();
    }

    void WindowsAudio::PlaySound(SoundEffect effect)
    {
        const std::filesystem::path path = GetEffectPath(effect);
        std::error_code fileError;
        if (!std::filesystem::exists(path, fileError) || fileError)
        {
            return;
        }

        ::PlaySoundW(
            path.c_str(),
            nullptr,
            SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
    }

    void WindowsAudio::StopSounds()
    {
        ::PlaySoundW(nullptr, nullptr, 0);
    }

    std::filesystem::path WindowsAudio::GetExecutableDirectory()
    {
        std::array<wchar_t, MAX_PATH> pathBuffer{};
        const DWORD length = GetModuleFileNameW(
            nullptr,
            pathBuffer.data(),
            static_cast<DWORD>(pathBuffer.size()));
        if (length == 0 || length >= pathBuffer.size())
        {
            std::error_code pathError;
            const std::filesystem::path currentPath =
                std::filesystem::current_path(pathError);
            return pathError ? std::filesystem::path(L".") : currentPath;
        }
        return std::filesystem::path(
            std::wstring(pathBuffer.data(), length)).parent_path();
    }

    std::filesystem::path WindowsAudio::GetMusicPath(MusicTrack track) const
    {
        switch (track)
        {
        case MusicTrack::Title:
            return audioDirectory_ / L"Bgm" / L"title.wav";
        case MusicTrack::Forge:
            return audioDirectory_ / L"Bgm" / L"forge.wav";
        case MusicTrack::Forging:
            return audioDirectory_ / L"Bgm" / L"forging.wav";
        case MusicTrack::BattleEmber:
            return audioDirectory_ / L"Bgm" / L"battle_ember.wav";
        case MusicTrack::BattleStorm:
            return audioDirectory_ / L"Bgm" / L"battle_storm.wav";
        case MusicTrack::BattleMemory:
            return audioDirectory_ / L"Bgm" / L"battle_memory.wav";
        case MusicTrack::Ending:
            return audioDirectory_ / L"Bgm" / L"ending.wav";
        }
        assert(false && "지원하지 않는 배경음악이다.");
        return {};
    }

    std::filesystem::path WindowsAudio::GetEffectPath(SoundEffect effect) const
    {
        switch (effect)
        {
        case SoundEffect::MenuMove:
            return audioDirectory_ / L"Sfx" / L"menu_move.wav";
        case SoundEffect::MenuConfirm:
            return audioDirectory_ / L"Sfx" / L"menu_confirm.wav";
        case SoundEffect::MenuBack:
            return audioDirectory_ / L"Sfx" / L"menu_back.wav";
        case SoundEffect::ForgeBegin:
            return audioDirectory_ / L"Sfx" / L"forge_begin.wav";
        case SoundEffect::Cool:
            return audioDirectory_ / L"Sfx" / L"cool.wav";
        case SoundEffect::Stoke:
            return audioDirectory_ / L"Sfx" / L"stoke.wav";
        case SoundEffect::HammerStrike:
            return audioDirectory_ / L"Sfx" / L"hammer_strike.wav";
        case SoundEffect::ForgeSuccess:
            return audioDirectory_ / L"Sfx" / L"forge_success.wav";
        case SoundEffect::ForgeFailure:
            return audioDirectory_ / L"Sfx" / L"forge_failure.wav";
        case SoundEffect::ForgeCritical:
            return audioDirectory_ / L"Sfx" / L"forge_critical.wav";
        case SoundEffect::PlayerAttack:
            return audioDirectory_ / L"Sfx" / L"player_attack.wav";
        case SoundEffect::Guard:
            return audioDirectory_ / L"Sfx" / L"guard.wav";
        case SoundEffect::PerfectGuard:
            return audioDirectory_ / L"Sfx" / L"perfect_guard.wav";
        case SoundEffect::PlayerHit:
            return audioDirectory_ / L"Sfx" / L"player_hit.wav";
        case SoundEffect::BattleVictory:
            return audioDirectory_ / L"Sfx" / L"battle_victory.wav";
        case SoundEffect::BattleDefeat:
            return audioDirectory_ / L"Sfx" / L"battle_defeat.wav";
        case SoundEffect::RewardConfirm:
            return audioDirectory_ / L"Sfx" / L"reward_confirm.wav";
        }
        assert(false && "지원하지 않는 효과음이다.");
        return {};
    }
}
