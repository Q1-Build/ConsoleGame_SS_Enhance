#pragma once

namespace ss
{
    // 기존 게임 장면은 이 뷰포트 안에서 좌표와 중앙 정렬을 그대로 유지한다.
    inline constexpr int kGameViewportWidth = 104;
    inline constexpr int kGameViewportHeight = 34;

    // 오른쪽 조작 안내와 아래 가상 키보드를 포함한 전체 발표용 프레임 크기다.
    inline constexpr int kScreenWidth = 174;
    inline constexpr int kScreenHeight = 48;

    inline constexpr float kPi = 3.1415926535f;
    inline constexpr unsigned long kTargetFrameMilliseconds = 16;
}
