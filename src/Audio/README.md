# Audio

Audio playback built on the vendored single-header **miniaudio** (`lib/miniaudio/`, compiled in `miniaudio.cpp`).

- **AudioEngine** — static singleton (mirrors `PhysicsEngine`/`ScriptEngine`) owning miniaudio's `ma_engine`: device, node graph, resource manager, and 3D spatializer. `RuntimeStart`/`RuntimeStop` bracket play mode. Each frame it follows emitter entities' transforms for spatial sounds and reaps finished instances.
- **AudioClip** — CPU-side asset holding a whole file decoded to interleaved f32 PCM (by `Asset/AudioImporter`). One clip is shared by every instance playing it; each instance reads through its own cursor.
- **AudioInstance** — RAII handle around a single `ma_sound` (analogous to a `RigidBody`'s runtime body).

There is no audio component — playback is driven entirely from Lua via the global `Audio.Play(clip, entity, opts)` API (bound in `Scripting/Bindings/AudioBindings`).
