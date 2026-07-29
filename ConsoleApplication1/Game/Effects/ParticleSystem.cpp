#include "Game/Effects/ParticleSystem.h"

#include "Core/GameConstants.h"
#include "Core/IRandomProvider.h"
#include "Rendering/IScreen.h"

#include <algorithm>
#include <cmath>

namespace ss
{
    ParticleSystem::ParticleSystem(IRandomProvider& randomProvider)
        : randomProvider_(randomProvider)
    {
    }

    void ParticleSystem::Update(float deltaSeconds)
    {
        for (Particle& particle : particles_)
        {
            particle.life -= deltaSeconds;
            particle.x += particle.velocityX * deltaSeconds;
            particle.y += particle.velocityY * deltaSeconds;
            particle.velocityY += deltaSeconds * 5.0f;
        }

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

    void ParticleSystem::SpawnAmbientEmber()
    {
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
            const float lifeRatio = particle.life / particle.maxLife;
            const Color color = lifeRatio < 0.28f ? Color::BrightBlack : particle.color;
            screen.Put(
                static_cast<int>(std::round(particle.x)),
                static_cast<int>(std::round(particle.y)),
                particle.glyph,
                color);
        }
    }
}
