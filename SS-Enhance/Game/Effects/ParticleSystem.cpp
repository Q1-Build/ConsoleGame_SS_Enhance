#include "Game/Effects/ParticleSystem.h"

#include "Core/GameConstants.h"
#include "Core/IRandomProvider.h"
#include "Rendering/IScreen.h"

#include <algorithm>
#include <cassert>
#include <cmath>

namespace ss
{
    ParticleSystem::ParticleSystem(IRandomProvider& randomProvider)
        : randomProvider_(randomProvider)
    {
    }

    void ParticleSystem::Update(float deltaSeconds)
    {
        // 파티클은 간단한 중력만 적용해 문자 애니메이션 비용을 낮게 유지한다.
        for (Particle& particle : particles_)
        {
            particle.life -= deltaSeconds;
            particle.x += particle.velocityX * deltaSeconds;
            particle.y += particle.velocityY * deltaSeconds;
            particle.velocityY += deltaSeconds * 5.0f;
        }

        // erase-remove로 만료된 파티클을 한 번에 정리해 반복 중 컨테이너 변경을 피한다.
        particles_.erase(
            std::remove_if(
                particles_.begin(),
                particles_.end(),
                [](const Particle& particle)
                {
                    return particle.life <= 0.0f;
                }),
            particles_.end());
    }

    void ParticleSystem::Clear() noexcept
    {
        particles_.clear();
    }

    void ParticleSystem::EmitAmbientEmbers(
        float deltaSeconds,
        float particlesPerSecond)
    {
        if (ShouldEmit(deltaSeconds, particlesPerSecond))
        {
            SpawnAmbientEmber();
        }
    }

    void ParticleSystem::EmitResultParticles(
        float deltaSeconds,
        float particlesPerSecond,
        bool succeeded,
        Color swordColor)
    {
        if (ShouldEmit(deltaSeconds, particlesPerSecond))
        {
            SpawnResultParticle(succeeded, swordColor);
        }
    }

    void ParticleSystem::SpawnAmbientEmber()
    {
        // 화면 아래의 임의 위치에서 위로 떠오르는 작은 불씨를 만든다.
        Particle particle;
        particle.x = randomProvider_.NextFloat(6.0f, static_cast<float>(kScreenWidth - 7));
        particle.y = static_cast<float>(kScreenHeight - 3);
        particle.velocityX = randomProvider_.NextFloat(-1.2f, 1.2f);
        particle.velocityY = randomProvider_.NextFloat(-7.0f, -2.5f);
        particle.life = randomProvider_.NextFloat(1.0f, 2.7f);
        particle.maxLife = particle.life;
        particle.glyph = randomProvider_.NextFloat(0.0f, 1.0f) > 0.6f ? L'*' : L'·';
        particle.color = randomProvider_.NextFloat(0.0f, 1.0f) > 0.5f
            ? Color::BrightRed
            : Color::BrightYellow;
        particles_.push_back(particle);
    }

    void ParticleSystem::SpawnImpact(float score)
    {
        // 정확한 타격일수록 더 많은 불꽃을 방사해 플레이 결과를 즉시 시각화한다.
        const int particleCount = 28 + static_cast<int>(score * 35.0f);
        for (int index = 0; index < particleCount; ++index)
        {
            Particle particle;
            particle.x = 52.0f;
            particle.y = 16.0f;

            const float angle = randomProvider_.NextFloat(kPi * 1.08f, kPi * 1.92f);
            const float speed = randomProvider_.NextFloat(12.0f, 38.0f);
            particle.velocityX = std::cos(angle) * speed;
            particle.velocityY = std::sin(angle) * speed * 0.45f;
            particle.life = randomProvider_.NextFloat(0.35f, 1.05f);
            particle.maxLife = particle.life;
            particle.glyph = index % 4 == 0 ? L'✦' : L'*';
            particle.color = index % 3 == 0
                ? Color::BrightWhite
                : (index % 2 == 0 ? Color::BrightYellow : Color::BrightRed);
            particles_.push_back(particle);
        }
    }

    void ParticleSystem::SpawnResultBurst(bool succeeded, Color swordColor, int count)
    {
        for (int index = 0; index < count; ++index)
        {
            SpawnResultParticle(succeeded, swordColor);
        }
    }

    void ParticleSystem::SpawnResultParticle(bool succeeded, Color swordColor)
    {
        // 성공은 검 색상의 별빛, 실패는 어두운 재로 서로 다른 감정을 표현한다.
        Particle particle;
        particle.x = 52.0f + randomProvider_.NextFloat(-5.0f, 5.0f);
        particle.y = 18.0f + randomProvider_.NextFloat(-2.0f, 2.0f);

        const float angle = randomProvider_.NextFloat(0.0f, kPi * 2.0f);
        const float speed = randomProvider_.NextFloat(3.0f, 16.0f);
        particle.velocityX = std::cos(angle) * speed;
        particle.velocityY = std::sin(angle) * speed * 0.45f - 2.0f;
        particle.life = randomProvider_.NextFloat(0.5f, 1.6f);
        particle.maxLife = particle.life;
        particle.glyph = succeeded
            ? (randomProvider_.NextFloat(0.0f, 1.0f) > 0.6f ? L'✦' : L'*')
            : L'·';
        particle.color = succeeded ? swordColor : Color::BrightBlack;
        particles_.push_back(particle);
    }

    void ParticleSystem::Draw(IScreen& screen) const
    {
        for (const Particle& particle : particles_)
        {
            // 수명이 거의 끝난 파티클은 어둡게 표시해 자연스럽게 사라지는 효과를 낸다.
            const float lifeRatio = particle.life / particle.maxLife;
            const Color color = lifeRatio < 0.28f ? Color::BrightBlack : particle.color;
            screen.Put(
                static_cast<int>(std::round(particle.x)),
                static_cast<int>(std::round(particle.y)),
                particle.glyph,
                color);
        }
    }

    bool ParticleSystem::ShouldEmit(
        float deltaSeconds,
        float particlesPerSecond)
    {
        assert(deltaSeconds >= 0.0f);
        assert(particlesPerSecond >= 0.0f);
        if (deltaSeconds <= 0.0f || particlesPerSecond <= 0.0f)
        {
            return false;
        }

        // 초당 생성률을 프레임 확률로 바꿔 실행 속도가 달라도 평균 밀도를 유지한다.
        const float emissionChance = std::min(
            1.0f,
            deltaSeconds * particlesPerSecond);
        return randomProvider_.NextFloat(0.0f, 1.0f) < emissionChance;
    }
}
