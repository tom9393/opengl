# Render

The rendering pipeline: a render graph over a thin OpenGL abstraction.

## Frame flow

1. `Scene/SceneExtractor` flattens the scene into a **RenderView** — camera data, opaque/transparent `DrawCommand` lists, particle batches, shadow casters, light data, environment, and post-process settings. The graph consumes this; passes never touch the ECS.
2. **RenderGraph** executes its passes in dependency order, binding each pass's framebuffer, applying its `PipelineState` (depth test/write, blend), clearing declared attachments, and timing it with GPU timer queries (shown in `StatsPanel`).

## Render graph

Passes declare named input/output resources; the graph resolves dependencies via topological sort, culls unused passes, and aborts on misdeclared graphs at compile. It adds write-after-read edges so readers of an aliased resource are ordered before an in-place writer (used by particles and bloom upsampling). The graph owns per-resource `RenderTarget` textures and composes each pass's `Framebuffer` from them at compile time; texture ids stay stable across viewport resizes (storage respecified in place). Passes may declare a viewport-relative size scale (bloom mips).

Pass order (see [Passes/](Passes/)): GeometryPass → ShadowPass2D/3D → ScenePass → ParticlePass → Bloom down/upsample → PostProcessPass.

## Low-level API

- **Renderer / RenderCommand / RendererAPI** — `Renderer` is the static low-level drawing API; `RendererAPI` is the abstract interface implemented by `platform/OpenGL/OpenGLRendererAPI`.
- **Shader** — GLSL program with load-time introspection (uniforms, UBOs, SSBOs); material properties bind automatically via this reflection.
- **Buffer / BufferLayout** — vertex/index buffer abstractions.
- **DynamicBufferPool** — persistently-mapped, coherent ring buffer split across frames in flight; per-draw instance/uniform data is sub-allocated from it (`RenderContext.InstancePool`).
- **RenderTexture** — sampleable texture owning an internal framebuffer, bound one face/mip at a time; used by offscreen sub-renderers needing cube faces or mip levels.
- **EnvironmentMap** — bakes image-based lighting from an equirectangular `.hdr`: environment cubemap, diffuse irradiance map, roughness-mipped specular prefilter, and a once-built shared BRDF LUT. Goes through `RenderCommand`/`RenderTexture`, not direct GL.
- **Camera / SceneCamera / EditorCamera** — projection-only base, the scene's camera component data, and the editor's orbiting fly camera.
- **RenderView / DrawCommand / ShadowCasters / Light / RenderItems** — the plain data structs the extraction step fills in.
- **Context** — swap-chain abstraction implemented by the platform layer.

`Scene/SceneRenderer` owns the graph instance, feeds it the `RenderView`, and debounces viewport resizes until the size settles.
