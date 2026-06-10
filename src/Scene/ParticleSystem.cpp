#include "ParticleSystem.h"
#include "Components.h"
#include "Scene.h"
#include "Core/TimeStep.h"

namespace Engine {

    void ParticleSystem::Play()
    {
        m_Playing = true;
        m_BurstFired = false;
    }

    void ParticleSystem::Update(const ParticleEmitterComponent& settings, const glm::mat4& emitterTransform, float dt)
    {
        if (m_Playing)
        {
            if (settings.Looping)
            {
                m_EmitAccumulator += settings.Rate * dt;
                uint32_t count = (uint32_t)m_EmitAccumulator;
                m_EmitAccumulator -= (float)count;
                Spawn(count, settings, emitterTransform);
            }
            else if (!m_BurstFired)
            {
                Spawn(settings.BurstCount, settings, emitterTransform);
                m_BurstFired = true;
            }
        }

        if (m_PendingEmit > 0)
        {
            Spawn(m_PendingEmit, settings, emitterTransform);
            m_PendingEmit = 0;
        }

        for (auto& p : m_Particles)
        {
            p.Velocity += settings.Gravity * dt;
            p.Position += p.Velocity * dt;
            p.Age += dt;
        }

        m_Particles.erase(
            std::remove_if(m_Particles.begin(), m_Particles.end(),
                           [](const Particle& p) { return p.Age >= p.Lifetime; }),
            m_Particles.end());
    }

    void ParticleSystem::Spawn(uint32_t count, const ParticleEmitterComponent& settings, const glm::mat4& emitterTransform)
    {
        if (m_Particles.size() >= settings.MaxParticles) return;
        count = std::min<uint32_t>(count, settings.MaxParticles - (uint32_t)m_Particles.size());
        if (count == 0) return;

        glm::vec3 origin = glm::vec3(emitterTransform[3]);
        glm::vec3 axis = glm::mat3(emitterTransform) * settings.Direction;
        axis = glm::dot(axis, axis) > 1e-12f ? glm::normalize(axis) : glm::vec3(0.0f, 1.0f, 0.0f);

        // Orthonormal basis around the emission axis.
        glm::vec3 helper = std::abs(axis.y) < 0.99f ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 tangent = glm::normalize(glm::cross(helper, axis));
        glm::vec3 bitangent = glm::cross(axis, tangent);

        std::uniform_real_distribution<float> unit(0.0f, 1.0f);
        float cosSpread = std::cos(glm::radians(settings.Spread));

        for (uint32_t i = 0; i < count; i++)
        {
            // Uniformly distributed direction inside the spread cone.
            float cosTheta = cosSpread + (1.0f - cosSpread) * unit(m_Rng);
            float sinTheta = std::sqrt(std::max(0.0f, 1.0f - cosTheta * cosTheta));
            float phi = 6.28318530718f * unit(m_Rng);

            glm::vec3 dir = axis * cosTheta + (tangent * std::cos(phi) + bitangent * std::sin(phi)) * sinTheta;

            Particle p;
            p.Position = origin;
            p.Velocity = dir * settings.Speed;
            p.Age = 0.0f;
            p.Lifetime = settings.Lifetime;
            m_Particles.push_back(p);
        }
    }

    void ParticleEngine::OnUpdate(Scene* scene, TimeStep ts)
    {
        if (!scene) return;

        auto view = scene->GetRegistry().view<TransformComponent, ParticleEmitterComponent>();
        for (auto e : view)
        {
            auto [transform, emitter] = view.get<TransformComponent, ParticleEmitterComponent>(e);
            if (!emitter.Runtime) emitter.Runtime = MakeShared<ParticleSystem>();
            emitter.Runtime->Update(emitter, transform.WorldTransform, ts);
        }
    }

}
