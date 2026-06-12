# Core

Application skeleton and platform-facing abstractions.

- **Application** — owns the `Window`, `LayerStack`, `ThreadPool`, and `ImGuiLayer`; `Run()` is the main loop (poll events → update layers → render UI → swap). `Editor` and `Runtime` are thin subclasses that push different layers. `Application::Get()` is the global accessor.
- **Layer / LayerStack** — units of update/event handling pushed onto the application. Layers receive `OnUpdate(TimeStep)` and events in stack order; overlays (like ImGui) sit on top.
- **Window** — abstract window interface (size, vsync, cursor capture, native handle, event callback). Implemented per platform in `platform/Linux/`.
- **Input / KeyCodes / MouseButtonCodes** — static polling-style input query API and the engine's key/mouse-button enums.
- **TimeStep** — frame delta-time value passed through updates.
- **UUID** — 64-bit random ids used to identify entities and assets persistently.
- **Preprocessor** — shared macros (bit flags, event-type boilerplate, etc.).
