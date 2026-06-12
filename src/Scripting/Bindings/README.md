# Scripting/Bindings

Per-module Sol2 registrars installed by `LuaBindings::Register`. One file per API surface:

- **MathBindings** — glm `vec2`/`vec3`/`vec4`/`quat` usertypes plus free helpers (`lerp`, `slerp`, `lookRotation`, `angleAxis`, …).
- **InputBindings** — the `Input` table plus `Key`/`Mouse` enums.
- **TransformBindings / TagBindings / RigidBodyBindings / CameraBindings / LightBindings / MaterialBindings / ParticleBindings** — one file per component usertype, exposed to scripts as bound handles (`Transform`, `RigidBody`, `Particles`, …).
- **EntityBindings** — the `Entity` usertype.
- **SceneBindings** — `Scene`/`RaycastResult` usertypes: `Scene.Raycast`, `Scene.Instantiate`, screen/world conversions, scene loading.
- **AudioBindings** — the `AudioInstance` usertype and global `Audio` table (`Audio.Play(clip, entity, opts)`).
