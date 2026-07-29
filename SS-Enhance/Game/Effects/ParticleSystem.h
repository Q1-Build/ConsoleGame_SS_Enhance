#pragma once

#include "Game/Effects/Particle.h"

#include <vector>

namespace ss
{
    class IRandomProvider;
    class IScreen;

    /// 모든 파티클의 생성, 물리 갱신, 제거와 화면 출력을 담당한다.
    class ParticleSystem final
    {
    public:
        explicit ParticleSystem(IRandomProvider& randomProvider);

        /// 모든 파티클을 경과 시간만큼 이동시키고 수명이 끝난 항목을 제거한다.
        void Update(float deltaSeconds);
        void Clear() noexcept;

        /// 프레임 속도와 무관한 초당 생성률로 배경 불씨를 방출한다.
        /// @param deltaSeconds 이전 프레임 이후 경과한 초
        /// @param particlesPerSecond 목표로 하는 초당 평균 생성 개수
        void EmitAmbientEmbers(float deltaSeconds, float particlesPerSecond);

        /// 프레임 속도와 무관한 초당 생성률로 결과 파티클을 방출한다.
        /// 성공 여부와 검 색상은 실제로 파티클이 생성되는 경우에만 표현에 사용된다.
        void EmitResultParticles(
            float deltaSeconds,
            float particlesPerSecond,
            bool succeeded,
            Color swordColor);

        /// 장면의 확정된 사건에 맞는 파티클 묶음을 즉시 생성한다.
        void SpawnImpact(float score);
        void SpawnResultBurst(bool succeeded, Color swordColor, int count);

        /// 현재 파티클을 화면에 그리며 파티클 상태는 변경하지 않는다.
        void Draw(IScreen& screen) const;

    private:
        [[nodiscard]] bool ShouldEmit(
            float deltaSeconds,
            float particlesPerSecond);
        void SpawnAmbientEmber();
        void SpawnResultParticle(bool succeeded, Color swordColor);

        // 난수 공급자는 애플리케이션보다 짧게 살지 않는 비소유 참조다.
        IRandomProvider& randomProvider_;
        std::vector<Particle> particles_;
    };
}
