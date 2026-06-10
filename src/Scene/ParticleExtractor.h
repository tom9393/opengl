#pragma once

#include "Asset/AssetManager.h"
#include "Asset/Texture.h"
#include "Components.h"
#include "ParticleSystem.h"
#include "Render/RenderContext.h"
#include "Render/RenderView.h"
#include "Scene.h"

namespace Engine {

class ParticleExtractor {
public:
    static void Extract(Scene* scene, RenderContext& ctx, RenderView& renderView)
    {
        auto view = scene->GetRegistry().view<TransformComponent, ParticleEmitterComponent>();
        for (auto e : view)
        {
            auto [transform, emitter] = view.get<TransformComponent, ParticleEmitterComponent>(e);
            if (!emitter.Runtime) continue;

            const std::vector<Particle>& particles = emitter.Runtime->GetParticles();
            if (particles.empty()) continue;

            ParticleBatch batch;
            batch.Additive = emitter.Additive;
            batch.EmitterPosition = glm::vec3(transform.WorldTransform[3]);
            if (emitter.TextureHandle)
            {
                // Only resolve once loaded; the no-op callback just kicks off the
                // async load (it may fire frames later, so it must not capture
                // anything from this stack frame). Untextured until then.
                if (AssetManager::IsAssetLoaded(emitter.TextureHandle))
                    batch.Texture = AssetManager::GetAsset<Texture2D>(emitter.TextureHandle);
                else
                    AssetManager::GetAssetAsync(emitter.TextureHandle, [](Shared<Asset>) {});
            }

            batch.Instances.reserve(particles.size());
            for (const Particle& p : particles)
            {
                float t = glm::clamp(p.Age / p.Lifetime, 0.0f, 1.0f);

                ParticleInstance inst;
                inst.Position = p.Position;
                inst.Size = glm::mix(emitter.StartSize, emitter.EndSize, t);
                inst.Color = glm::mix(emitter.StartColor, emitter.EndColor, t);
                batch.Instances.push_back(inst);
            }

            // Alpha blending needs back-to-front ordering within the batch.
            if (!batch.Additive)
            {
                glm::vec3 cam = ctx.CameraPosition;
                std::sort(batch.Instances.begin(), batch.Instances.end(),
                    [&cam](const ParticleInstance& a, const ParticleInstance& b) {
                        glm::vec3 da = a.Position - cam;
                        glm::vec3 db = b.Position - cam;
                        return glm::dot(da, da) > glm::dot(db, db);
                    });
            }

            renderView.ParticleBatches.push_back(std::move(batch));
        }

        // Alpha batches first (farthest emitter first), additive last: additive is
        // order-independent and must not be occluded by alpha quads drawn over it.
        glm::vec3 cam = ctx.CameraPosition;
        std::sort(renderView.ParticleBatches.begin(), renderView.ParticleBatches.end(),
            [&cam](const ParticleBatch& a, const ParticleBatch& b) {
                if (a.Additive != b.Additive) return !a.Additive;
                glm::vec3 da = a.EmitterPosition - cam;
                glm::vec3 db = b.EmitterPosition - cam;
                return glm::dot(da, da) > glm::dot(db, db);
            });
    }
};

}
