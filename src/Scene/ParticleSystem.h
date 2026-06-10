#pragma once

#include "pch.h"

namespace Engine {

    struct ParticleEmitterComponent;
    class Scene;
    class TimeStep;

    struct Particle {
        glm::vec3 Position;
        glm::vec3 Velocity;
        float Age = 0.0f;
        float Lifetime = 1.0f;
    };

    // Per-emitter runtime state (RigidBodyComponent::RuntimeBody pattern): owns the
    // live particle pool. Created lazily by ParticleEngine, never serialized.
    class ParticleSystem {
    public:
        void Update(const ParticleEmitterComponent& settings, const glm::mat4& emitterTransform, float dt);

        // Emission requests are deferred to the next Update, where the emitter's
        // settings and transform are available (callers like Lua only hold the system).
        void Play();
        void Stop() { m_Playing = false; }
        void Emit(uint32_t count) { m_PendingEmit += count; }
        bool IsPlaying() const { return m_Playing; }

        const std::vector<Particle>& GetParticles() const { return m_Particles; }

    private:
        void Spawn(uint32_t count, const ParticleEmitterComponent& settings, const glm::mat4& emitterTransform);

        std::vector<Particle> m_Particles;
        float m_EmitAccumulator = 0.0f;
        uint32_t m_PendingEmit = 0;
        bool m_Playing = true;
        bool m_BurstFired = false; // one-shot emitters burst once per Play()
        std::mt19937 m_Rng{ std::random_device{}() };
    };

    // Steps every emitter in a scene; called from Scene::OnUpdate (play mode) and
    // the editor's edit-mode update (live preview).
    class ParticleEngine {
    public:
        static void OnUpdate(Scene* scene, TimeStep ts);
    };

}
