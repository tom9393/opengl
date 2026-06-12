# src/

Engine and editor source. The entry point instantiates either `Editor` or `Runtime` (both `Application` subclasses from `Core/`) and calls `Run()`, which drives a `LayerStack`. The two root-level layers live here:

- **EditorLayer** — the full editor: owns the active scene, the panels (`Panels/`), play/edit mode switching, and per-frame scene + render updates.
- **ImGuiLayer** — ImGui platform integration; begins/ends the UI frame that all panels render into.

## Subsystem map

| Folder | Responsibility |
|--------|----------------|
| [Core/](Core/) | Application loop, layers, window/input abstractions |
| [Events/](Events/) | Typed publish-subscribe event system |
| [Asset/](Asset/) | Asset registry, importers, async loading, materials, model import |
| [Scene/](Scene/) | EnTT-based ECS, components, serialization, render extraction |
| [Render/](Render/) | Render graph, passes, shaders, low-level GL abstraction |
| [Modeling/](Modeling/) | Half-edge mesh editing (modeling + UV) |
| [Commands/](Commands/) | Undo/redo command stack |
| [Physics/](Physics/) | Bullet3 wrapper, rigid bodies, debug drawing |
| [Scripting/](Scripting/) | Native C++ scripts and Lua (Sol2) bindings |
| [Audio/](Audio/) | miniaudio-based audio engine and playback |
| [Panels/](Panels/) | Editor UI panels (viewport, hierarchy, content browser, …) |
| [Utils/](Utils/) | Cross-cutting helpers (thread pool, math, editor state) |
