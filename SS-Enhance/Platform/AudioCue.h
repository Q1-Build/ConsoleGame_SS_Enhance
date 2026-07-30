#pragma once

namespace ss
{
    /// 장면이 요청할 수 있는 반복 배경음악을 플랫폼 파일 이름과 분리해 표현한다.
    enum class MusicTrack
    {
        Title,
        Forge,
        Forging,
        BattleEmber,
        BattleStorm,
        BattleMemory,
        Ending
    };

    /// 게임 사건에 한 번 재생할 효과음을 플랫폼 파일 이름과 분리해 표현한다.
    enum class SoundEffect
    {
        MenuMove,
        MenuConfirm,
        MenuBack,
        ForgeBegin,
        Cool,
        Stoke,
        HammerStrike,
        ForgeSuccess,
        ForgeFailure,
        ForgeCritical,
        PlayerAttack,
        Guard,
        PerfectGuard,
        PlayerHit,
        BattleVictory,
        BattleDefeat,
        RewardConfirm
    };
}
