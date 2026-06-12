#include "SceneBindings.h"

#include "pch.h"
#include "Asset/AssetManager.h"
#include "Asset/Prefab.h"
#include "Physics/PhysicsEngine.h"
#include "Physics/PhysicsQueries.h"
#include "Scene/Components.h"
#include "Scene/Entity.h"
#include "Scene/Scene.h"
#include "Utils/SceneManager.h"

namespace Engine {

    // Matrices and viewport of the primary camera, as used by SceneRenderer::RenderRuntime,
    // for converting between viewport pixels and world space. False if the scene
    // has no usable primary camera (or its viewport size is not known yet).
    static bool GetPrimaryCameraData(Scene& scene, glm::mat4& outViewProjection, glm::vec2& outViewport)
    {
        auto view = scene.GetRegistry().view<TransformComponent, SceneCameraComponent>();
        for (auto e : view)
        {
            auto [transform, camera] = view.get<TransformComponent, SceneCameraComponent>(e);
            if (!camera.primary || !camera.sceneCamera) continue;

            outViewProjection = camera.sceneCamera->GetProjection() * glm::inverse(transform.GetTransform());
            outViewport = { camera.sceneCamera->GetViewportWidth(), camera.sceneCamera->GetViewportHeight() };
            return outViewport.x > 0.0f && outViewport.y > 0.0f;
        }
        return false;
    }

    void SceneBindings::Register(sol::state& state)
    {
        state.new_usertype<RaycastResult>("RaycastResult",
            "Hit",      &RaycastResult::Hit,
            // Wraps the raw handle so scripts get a full Entity (nil on a miss).
            "Entity",   sol::property([](RaycastResult& result, sol::this_state s) -> sol::object {
                if (!result.Hit || result.HitHandle == entt::null) return sol::nil;
                return sol::make_object(s, Entity{ result.HitHandle, PhysicsEngine::GetScene() });
            }),
            "Position", &RaycastResult::Position,
            "Normal",   &RaycastResult::Normal
        );

        state.new_usertype<Scene>("Scene",
        "CreateEntity", &Scene::CreateEntity,
        // First entity whose tag matches, or nil.
        "Find", [](Scene& scene, const std::string& name, sol::this_state s) -> sol::object {
            auto view = scene.GetRegistry().view<TagComponent>();
            for (auto e : view)
                if (view.get<TagComponent>(e).tag == name)
                    return sol::make_object(s, Entity{ e, &scene });
            return sol::nil;
        },
        // Scene.Load("name") -> switches to the named scene asset between frames,
        // never mid-update. The current scene is stopped, the new one started.
        "Load", [](Scene& scene, const std::string& name) {
            SceneManager::RequestSceneLoad(name);
        },
        // Optional 4th argument: an entity whose body the ray should pass through
        // (typically Self, so the caster doesn't hit its own collider).
        "Raycast", [](Scene& scene, const glm::vec3 origin, const glm::vec3 dir, float dist, sol::optional<Entity> ignore) {
            return PhysicsQueries::Raycast(origin, dir, dist, ignore ? ignore->GetHandle() : entt::null);
        },
        // Every body along the ray, nearest first, as a list of RaycastResults.
        "RaycastAll", [](Scene& scene, const glm::vec3 origin, const glm::vec3 dir, float dist) {
            return PhysicsQueries::RaycastAll(origin, dir, dist);
        },
        // Entities whose bodies overlap the sphere (e.g. blast radius queries).
        "OverlapSphere", [](Scene& scene, const glm::vec3 center, float radius) {
            std::vector<Entity> entities;
            for (auto handle : PhysicsQueries::OverlapSphere(center, radius))
                entities.push_back(Entity{ handle, &scene });
            return entities;
        },
        // Scene:ScreenPointToRay(x, y) -> origin, direction (vec3s, or nil if there
        // is no primary camera). x/y are pixels in the viewport the scene renders
        // into, so Scene:ScreenPointToRay(Input.GetMousePosition()) gives the ray
        // under the mouse, ready to feed into Scene:Raycast.
        "ScreenPointToRay", [](Scene& scene, float x, float y, sol::this_state s) -> std::tuple<sol::object, sol::object> {
            glm::mat4 viewProjection; glm::vec2 viewport;
            if (!GetPrimaryCameraData(scene, viewProjection, viewport))
                return { sol::nil, sol::nil };

            glm::vec2 ndc = { 2.0f * x / viewport.x - 1.0f, 1.0f - 2.0f * y / viewport.y };
            glm::mat4 inverse = glm::inverse(viewProjection);
            glm::vec4 nearPoint = inverse * glm::vec4(ndc, -1.0f, 1.0f);
            glm::vec4 farPoint  = inverse * glm::vec4(ndc,  1.0f, 1.0f);
            nearPoint /= nearPoint.w;
            farPoint  /= farPoint.w;

            glm::vec3 origin = glm::vec3(nearPoint);
            glm::vec3 direction = glm::normalize(glm::vec3(farPoint - nearPoint));
            return { sol::make_object(s, origin), sol::make_object(s, direction) };
        },
        // World position -> viewport pixels (vec2), or nil if behind the camera.
        "WorldToScreen", [](Scene& scene, const glm::vec3& position, sol::this_state s) -> sol::object {
            glm::mat4 viewProjection; glm::vec2 viewport;
            if (!GetPrimaryCameraData(scene, viewProjection, viewport)) return sol::nil;

            glm::vec4 clip = viewProjection * glm::vec4(position, 1.0f);
            if (clip.w <= 0.0f) return sol::nil;

            glm::vec3 ndc = glm::vec3(clip) / clip.w;
            glm::vec2 pixel = { (ndc.x * 0.5f + 0.5f) * viewport.x, (1.0f - (ndc.y * 0.5f + 0.5f)) * viewport.y };
            return sol::make_object(s, pixel);
        },
        // Scene.Instantiate("prefabName", position, rotation) -> Entity.
        // position/rotation are optional vec3s (rotation is euler radians).
        "Instantiate", [](Scene& scene, const std::string& name, sol::optional<glm::vec3> position, sol::optional<glm::vec3> rotation) -> Entity {
            UUID handle = AssetManager::GetHandleByName(name, AssetType::Prefab);
            if (handle == 0) {
                std::cerr << "Scene.Instantiate: no prefab named '" << name << "'" << std::endl;
                return Entity{};
            }
            Shared<Prefab> prefab = AssetManager::GetAsset<Prefab>(handle);
            glm::vec3 p = position ? *position : glm::vec3(0.0f);
            glm::vec3 r = rotation ? *rotation : glm::vec3(0.0f);
            return scene.InstantiatePrefab(prefab, p, r);
        });
    }

}
