# Scene

The ECS world, built on **EnTT**.

- **Scene** — owns the `entt::registry`, scene settings (environment, post-process), entity creation/destruction/duplication, the parent/child hierarchy, and the play-mode lifecycle (`OnUpdate` steps scripts, physics, and particles).
- **Entity** — thin handle (entity id + scene pointer) with typed component access.
- **Components** — all data lives here: `TransformComponent`, `MeshComponent`, `MaterialComponent`, `SceneCameraComponent`, `LightComponent`, `RigidBodyComponent`, `ParticleEmitterComponent`, scripting (`NativeScriptComponent`, `LuaScriptComponent`), and `ParentComponent`/`ChildrenComponent` for the hierarchy.

## Rendering bridge

- **SceneRenderer** — owns the render graph, feeds it the extracted view each frame, and debounces viewport resizes until the size settles.
- **SceneExtractor** — flattens the scene into a `Render/RenderView`; delegates to **MeshExtractor** (draw commands + shadow casters, with frustum culling), **LightExtractor** (light data), and **ParticleExtractor** (per-emitter `ParticleBatch`es, sorting alpha-blended ones back-to-front while additive batches draw last).

## Particles

- **ParticleSystem** — runtime particle pool lazily created by a `ParticleEmitterComponent` (same pattern as a rigid body's runtime body; never serialized). `ParticleEngine::OnUpdate` steps every emitter — from `Scene::OnUpdate` in play mode and from `EditorLayer`'s edit-mode update for live preview; runtime systems are nulled at play start so the play-mode scene copy gets fresh state.

## Serialization

- **SceneSerializer** — binary `.pmscene` editor scenes ("PSCN" magic): scene settings plus per-entity component data and asset-handle references, deserialized in two passes (create all entities, then relink parents). The same extension also carries baked imported models ("PMSC", see `Asset/PmSceneSerializer`).
- **PrefabSerializer / EntitySerializer** — YAML `.prefab` files: an entity subtree saved as component data + asset-handle references.
