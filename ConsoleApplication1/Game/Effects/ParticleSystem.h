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

        void Update(float deltaSeconds);
        void Clear() noexcept;
        void SpawnAmbientEmber();
        void SpawnImpact(float score);
        void SpawnResultBurst(bool succeeded, Color swordColor, int count);
        void SpawnResultParticle(bool succeeded, Color swordColor);
        void Draw(IScreen& screen) const;

    private:
        // 난수 공급자는 애플리케이션보다 짧게 살지 않는 비소유 참조다.
        IRandomProvider& randomProvider_;
        std::vector<Particle> particles_;
    };
}
