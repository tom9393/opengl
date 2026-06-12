# Asset

Asset registry, importers, and async loading.

- **AssetManager** — static singleton holding the central registry (metadata persisted in `assets/AssetRegistry.yaml`). `GetAssetInternal(Async)` dispatches on `AssetType` to the per-type importer; async loads run on the `ThreadPool` and GPU uploads are deferred to the main thread through **AssetUploadQueue**.
- **Asset** — base class for all resource types (`Mesh`, `Texture2D`, `Material`, `Scene`, `Prefab`, `AudioClip`), each identified by a UUID handle.

## Importers

- **TextureImporter** — images via stb_image / KTX, plus `LoadHDR` for float equirectangular maps. **DefaultTextures** provides built-in fallbacks (white, normal, …).
- **SceneImporter** — `.obj/.fbx/.gltf` models and `.pmscene` files. Model import is two-phase so heavy parsing stays off the main thread: **AssimpLoader** reads the file into a CPU-only **ParsedScene** tree (nodes with transforms/meshes/lights/cameras + a scene-global material list), then **SceneBuilder** runs on the main thread to upload GPU resources and assemble the entity hierarchy into a **Prefab**. AssimpLoader gives every mesh/material a deterministic FNV-1a hash as its asset handle, so re-imports reuse handles and shared resources dedupe.
- **PmSceneSerializer** — baked imported models: a binary `ParsedScene` with embedded geometry ("PMSC" magic). `.pmscene` carries two formats distinguished by magic — editor scenes ("PSCN", handled by `Scene/SceneSerializer`) and these baked models; `SceneImporter` dispatches on the magic.
- **PrefabImporter** — loads saved `.prefab` YAML assets via `Scene/PrefabSerializer`.
- **AudioImporter** — decodes audio files to CPU-side PCM `AudioClip`s.
- **ScriptImporter** — registers Lua script files as assets.

## Materials & misc

- **Material / MaterialSpecification / MatdefLoader** — PBR material asset; property layouts are declared in `.matdef` files in `shaders/` and bound to shader uniforms/SSBOs via reflection in `Shader`.
- **Mesh** — GPU mesh asset (vertex/index buffers + submeshes). **PrimitiveBuilder** generates built-in primitives (cube, sphere, plane, …).
- **CollisionBaker** — bakes an entity's mesh subtree into simplified convex hulls (`.pmcol` under `assets/collision/`), keyed by an AABB hash so identical geometry shares one bake; orphaned bakes are deleted.
- **YamlArchive** — yaml-cpp serialization helpers shared by the YAML-based serializers.
