# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Run

```bash
./configure.sh   # cmake -S . -B build (regenerate build files)
./build.sh       # make in build/ directory
./run.sh         # execute the compiled binary
```

System dependencies (Arch Linux): `pacman -Syu assimp yaml-cpp glfw`
Lua 5.4 required exactly — 5.5 is not supported.

There are no automated tests; validation is done through interactive editor/runtime use.

## Architecture

### Application Structure

The entry point (`main.cpp`) instantiates either `Editor` or `Runtime` and calls `Run()`. Both inherit from `Application`, which manages a `LayerStack`. The two key layers are `EditorLayer` (full UI + viewport) and `ImGuiLayer` (UI rendering). The platform layer (`platform/Linux/`, `platform/OpenGL/`) implements window management and the OpenGL rendering API behind abstract interfaces.

### Editor UI (Panels)

`EditorLayer` composes the editor from panels in `src/Panels/`:

- **Viewport** (`Viewport/`) — `ViewportPanel` hosts three modes that share `ViewportPanelBase` and a `ViewportContext` (scene, renderer, editor camera): `ObjectViewportPanel` (scene/object editing with gizmos), `ModelingViewportPanel` (mesh editing), and `UVViewportPanel` (UV editing). `PassPanel` selects which render-graph output is displayed.
- **SceneHierarchy** (`SceneHierarchy/`) — `SceneHierarchyPanel` plus per-component inspectors (`DrawTransform`, `DrawMesh`, `DrawMaterial`, `DrawLight`, `DrawRigidBody`, `DrawLua`, …).
- **ContentBrowser** (`ContentBrowser/`) — `ContentBrowserPanel` with async asset thumbnails rendered by `AssetPreviewManager` (`Texture2DPreview`, `MaterialPreview`, `PrefabPreview`).
- Standalone panels: `MenubarPanel`, `ToolbarPanel`, `StatsPanel`, `SettingsPanel`, `ShaderPanel`.

### Mesh Editing

Mesh editing lives in `src/Modeling/`. `HalfEdgeMesh` is a DCEL (half-edge) structure supporting n-gon faces over manifold + boundary surfaces, with in-place topology ops (`ExtrudeFaces`, `DeleteFaces`, `SubdivideFaces`) that snapshot the DCEL, splice it directly, then re-validate manifold invariants — restoring the snapshot if the result is broken. `MeshEditSession` bridges an entity's render `Mesh` and an editable `HalfEdgeMesh`: `Begin` clones the mesh into a unique in-memory asset (so edits never touch a shared/source asset) and welds render vertices into topological vertices, `SyncToMesh` triangulates + recomputes normals and re-uploads, and `End` commits the clone or restores the original handle. It carries parallel per-vertex attributes (UV/tangent) plus a per-corner UV layer (`UVVerts`/`CornerUV`) that represents seams/islands. `ModelingViewportPanel` drives topology edits; `UVViewportPanel` edits the per-corner UV layer.

### Undo/Redo (Commands)

`src/Commands/` implements undo/redo. `Command` is the base interface (`Undo`/`Redo`), and a command resolves what it needs (entities by UUID, the active scene) at Undo/Redo time rather than holding raw handles. `CommandHistory` owns the undo/redo stacks: `Execute` applies a freshly-built command and records it, `Push` records one whose effect was already applied live (e.g. a finished ImGui drag), with depth capped at 100. `CommandHistory::Global()` is the single shared history used for both scene and mesh edits. Concrete commands live in `SceneCommands.h` (`ModifyEntityCommand`, `CreateEntityCommand`, `DeleteEntityCommand`, `ReparentCommand` — built on an `EntitySnapshot` / subtree memento that preserves UUIDs), `MaterialEditCommand`, and `MeshEditCommand` (restores a whole `MeshEditSession::State` and re-syncs; the global history is cleared when the session ends).

### ECS & Scene

Scenes are built on **EnTT**. `Scene` owns the registry; `Entity` is a thin handle into it. All data lives in components defined in `src/Scene/Components.h` — `TransformComponent`, `MeshComponent`, `MaterialComponent`, `SceneCameraComponent`, `LightComponent`, `RigidBodyComponent`, the scripting components (`NativeScriptComponent`, `LuaScriptComponent`), and `ParentComponent`/`ChildrenComponent` for the parent/child hierarchy. `SceneRenderer` bridges the scene to the render graph.

### Render Graph

The rendering pipeline uses a true render graph (`src/Render/RenderGraph.h`) with named input/output resources, automatic dependency resolution via topological sort, and unused-pass culling. The main passes are:

- **GeometryPass** — deferred G-buffer generation
- **ShadowPass2D / ShadowPass3D** — directional and point light shadows
- **ScenePass** — lighting composition and forward rendering
- **PostProcessPass** — tone mapping plus bloom/exposure/vignette/chromatic-aberration, driven by the scene's `PostProcessSettings`

`Renderer` is a static low-level API; `RendererAPI` is the abstract interface implemented by `platform/OpenGL/OpenGLRendererAPI`.

Render-graph passes draw into fixed-size `Framebuffer` attachments. Offscreen sub-renderers that need cube faces or mip levels instead use `RenderTexture` (`src/Render/RenderTexture.h`) — a sampleable texture that owns an internal framebuffer and is bound one face/mip at a time. Both `Bloom` (the dual-filter mip chain) and `EnvironmentMap` build on it, going through `RenderCommand`/`RenderTexture` rather than direct GL. `EnvironmentMap` bakes image-based lighting from an equirectangular `.hdr` (loaded via `TextureImporter::LoadHDR` as a float texture): an environment cubemap, a diffuse irradiance map, a roughness-mipped specular prefilter map, and a once-built shared BRDF integration LUT.

### Asset System

`AssetManager` (static singleton) maintains a central registry and handles async loading via `ThreadPool`. All resource types (`Mesh`, `Texture2D`, `Material`, `Scene`, `Prefab`, `AudioClip`) inherit from `Asset`. `AssetManager::GetAssetInternal(Async)` dispatches on `AssetType` to per-type importers (`SceneImporter`, `PrefabImporter`, `TextureImporter`, `AudioImporter`, `ScriptImporter`). GPU uploads are batched through `AssetUploadQueue`. Asset metadata is stored in `assets/AssetRegistry.yaml`; textures support KTX format via the `lib/ktx` submodule.

Model import (`.obj/.fbx/.gltf`, registered as `AssetType::Scene`) is a two-phase pipeline so heavy parsing stays off the main thread: `SceneImporter::LoadScene` calls `AssimpLoader::LoadScene`, which reads the file via Assimp into a CPU-only `ParsedScene` tree (`ParsedScene.h` — `ParsedSceneNode`s carrying transforms, meshes, lights, cameras, plus a scene-global `ParsedMaterial` list), then `SceneBuilder::BuildPrefab` runs on the main thread to upload meshes/textures and assemble the entity hierarchy into a `Prefab` (a `Scene` subclass). For async loads, `LoadScene` runs on the thread pool and `BuildPrefab` is deferred to the main thread via `AssetUploadQueue`. `AssimpLoader` assigns each parsed mesh/material a deterministic FNV-1a `Hash` (from its name + vertex positions / texture data) that becomes the asset handle, so re-imports reuse the same handles and shared meshes/materials dedupe across loads.

Editor scenes are binary `.pmscene` files: `SceneSerializer` writes scene settings (environment, post-process) plus per-entity component data + asset-handle references, deserialized in two passes — create every entity, then relink parents. The `.pmscene` extension carries two formats distinguished by magic: editor scenes ("PSCN", `SceneSerializer`) and baked imported models ("PMSC", `PmSceneSerializer` — a binary `ParsedScene` with embedded geometry, loaded via `SceneBuilder::BuildPrefab`); `SceneImporter` dispatches on the magic. Prefab files are YAML (yaml-cpp): `PrefabSerializer` reads/writes `.prefab` files (an entity subtree saved as component data + asset-handle references) via `EntitySerializer`. Prefabs thus have two origins: imported models (built live by `SceneBuilder`) and saved `.prefab` assets (loaded by `PrefabImporter` via `PrefabSerializer`).

### Material System

`Material` (`src/Asset/Material.h`) is PBR-based, with blend modes and shader property binding. Properties are declared in `.matdef` files in `shaders/` and bound to shader uniforms/SSBOs automatically via reflection in `Shader`.

### Physics & Scripting

`PhysicsEngine` wraps Bullet3 and steps the world. A `RigidBodyComponent` carries a runtime `RigidBody` (`src/Physics/RigidBody.h`) — an RAII wrapper around a `btRigidBody` + motion state, built from an `RBShape` (`RBBoxShape` / `RBConvexShape` / `RBComplexShape` / `RBPlaneShape`) and exposing the gameplay API (impulses, velocities, kinematic toggles, etc.). `CollisionBaker` bakes an entity's mesh subtree into simplified convex hulls written to `.pmcol` files under `assets/collision/`, keyed by an FNV-1a hash of the meshes' AABBs so identical geometry shares one bake; bakes with no remaining referencing entity are deleted. `PhysicsDebugDrawer` visualizes collision shapes. Scripting has two paths: native C++ behaviors attached via `NativeScriptComponent`, and Lua scripts attached via `LuaScriptComponent`. The Lua path uses **Sol2** bindings over Lua 5.4 — `ScriptEngine` manages the VM and `LuaBindings::Register` installs the C++ API, split across per-module registrars in `src/Scripting/Bindings/`: `MathBindings` (glm `vec2`/`vec3`/`vec4`/`quat` usertypes plus free helpers like `lerp`/`slerp`/`lookRotation`/`angleAxis`), `InputBindings` (the `Input` table plus `Key`/`Mouse` enums), one file per component usertype (`TransformBindings`, `TagBindings`, `RigidBodyBindings`, `CameraBindings`, `LightBindings`, `MaterialBindings`), `EntityBindings` (the `Entity` usertype), `SceneBindings` (the `Scene`/`RaycastResult` usertypes — `Scene.Raycast`, `Scene.Instantiate`, screen/world conversions), and `AudioBindings` (the `AudioInstance` usertype and global `Audio` table). `LuaBindings` also owns the per-entity lifecycle: `BindEntity` gives each scripted entity its own `sol::environment` seeded with `Self`/`Scene` and bound component handles (`Transform`, `RigidBody`, …), runs the script file into it, and calls `OnCreate`.

### Audio

`src/Audio/` wraps the vendored single-header **miniaudio** (`lib/miniaudio/miniaudio.h`). `AudioEngine` is a static singleton (mirroring `PhysicsEngine`/`ScriptEngine`) owning miniaudio's `ma_engine` — device, node graph, resource manager, and 3D spatializer. An `AudioClip` is a CPU-side asset holding the whole file decoded to interleaved f32 PCM (decoded by `AudioImporter`); one clip is shared by every `AudioInstance` that plays it, each reading through its own cursor. `AudioInstance` is the RAII runtime handle around a single `ma_sound` (analogous to a `RigidBody`'s runtime body). There is **no audio component** — playback is driven entirely from Lua via the global `Audio.Play(clip, entity, opts)` API: the engine owns the live sound, follows the emitter entity's transform each frame for spatial sounds, and reaps finished sounds. `AudioEngine::RuntimeStart`/`RuntimeStop` bracket play mode.

### Event System

`EventBroker` implements publish-subscribe. Events are categorized (Keyboard, Mouse, Application, Window) and typed via `EventType` enum. Dispatch is type-safe.

## Key Dependencies (all in `lib/` as git submodules)

| Library | Role |
|---------|------|
| EnTT | Entity-component system |
| GLM | Math (vec, mat, quat) |
| ImGui + ImGuizmo | Editor UI and 3D gizmos |
| Bullet3 | Physics simulation |
| Sol2 | Lua C++ bindings |
| glad / GLFW | OpenGL loader / window & input |
| yaml-cpp | Prefab & asset-registry serialization |
| stb_image / ktx | Texture loading |
| miniaudio | Audio playback (vendored single header, not a submodule) |

Precompiled headers are in `pch.h` — include it rather than re-including common headers.

## Shaders

GLSL shaders live in `shaders/`. `.matdef` files define material property layouts. Shader introspection (uniforms, UBOs, SSBOs) is performed at load time by `Shader`.

## 1. Think Before Coding

**Don't assume. Don't hide confusion. Surface tradeoffs.**

Before implementing:
- State your assumptions explicitly. If uncertain, ask.
- If multiple interpretations exist, present them - don't pick silently.
- If a simpler approach exists, say so. Push back when warranted.
- If something is unclear, stop. Name what's confusing. Ask.

## 2. Simplicity First

**Minimum code that solves the problem. Nothing speculative.**

- Avoid clever one liners and other hacky solutions. Write code that is easy to understand and simple.
- No features beyond what was asked.
- No abstractions for single-use code.
- No "flexibility" or "configurability" that wasn't requested.
- No error handling for impossible scenarios.
- If you write 200 lines and it could be 50, rewrite it.

Ask yourself: "Would a senior engineer say this is overcomplicated?" If yes, simplify.

## 3. Surgical Changes

**Touch only what you must. Clean up only your own mess.**

When editing existing code:
- Don't "improve" adjacent code, comments, or formatting.
- Don't refactor things that aren't broken.
- Match existing style, even if you'd do it differently.
- If you notice unrelated dead code, mention it - don't delete it.

When your changes create orphans:
- Remove imports/variables/functions that YOUR changes made unused.
- Don't remove pre-existing dead code unless asked.

The test: Every changed line should trace directly to the user's request.

## 4. Do not manually compile

**When done, tell the user so they can test whether the program compiles manually**
