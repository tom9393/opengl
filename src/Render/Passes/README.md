# Render/Passes

The render graph's passes. Each subclasses **RenderPass**, declaring named input/output resources, a `PipelineState` (depth test/write, blend), per-attachment clear values, and optionally a viewport-relative size scale.

| Pass | Role |
|------|------|
| **GeometryPass** | Deferred G-buffer generation (albedo, normals, material params, depth). |
| **ShadowPass2D / ShadowPass3D** | Directional/cone and point light shadows, rendered layered (`gl_Layer` via geometry shaders) into array targets. |
| **ScenePass** | Lighting composition (PBR + IBL + shadows) plus forward rendering of transparents, writing `SceneColor`. |
| **ParticlePass** | Camera-facing instanced billboards blended in place on `SceneColor` (aliased as `SceneColorParticles`); depth-tests manually against `GBuffer.Depth` in the fragment shader. |
| **BloomPass** (`AddBloomPasses`) | Progressive dual-filter bloom: downsample chain over viewport-relative `Bloom.MipN` resources, then upsamples that blend additively in place (aliased `Bloom.UpN`). |
| **PostProcessPass** | Tone mapping plus exposure/vignette/chromatic aberration from the scene's `PostProcessSettings`; consumes `SceneColorParticles` and `Bloom.Up0`. |

In-place aliasing (particles, bloom upsample) relies on the graph's write-after-read edges to order readers before the writer. `Panels/PassPanel` selects which graph output the viewport displays.
