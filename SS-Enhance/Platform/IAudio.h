#pragma once

#include "Platform/AudioCue.h"

namespace ss
{
    /// 장면이 운영체제와 음원 파일 형식을 모르고 음악과 효과음을 요청하는 계약이다.
    class IAudio
    {
    public:
        virtual ~IAudio() = default;

        /// 지정한 배경음악을 반복 재생하며 이미 재생 중인 같은 곡은 다시 시작하지 않는다.
        virtual void PlayMusic(MusicTrack track) = 0;

        /// 현재 배경음악을 정지하고 관련 플랫폼 자원을 해제한다.
        virtual void StopMusic() = 0;

        /// 짧은 효과음을 한 번 비동기로 재생하며 재생 중인 효과음이 있으면 교체한다.
        virtual void PlaySound(SoundEffect effect) = 0;

        /// 현재 재생 중인 효과음만 정지하며 배경음악은 유지한다.
        virtual void StopSounds() = 0;
    };
}
