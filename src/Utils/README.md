# Utils

Cross-cutting helpers.

- **SceneManager** — static editor state: the active scene context, selected entity, edit/play `SceneState`, and a pending scene-load request (set from Lua's `Scene.Load`, consumed between frames so a running scene is never torn down mid-update).
- **GizmoManager** — thin ImGuizmo wrapper: viewport rect, current operation (translate/rotate/scale), and `Manipulate` with snapping.
- **ThreadPool** — worker pool used for async asset loading.
- **MathTools** — matrix decomposition plus `AABB`/`Plane`/`Frustum` types used for culling.
- **Instrumentor** — scoped CPU profiling macros writing chrome-tracing JSON.
- **PlatformUtils** — native open/save file dialogs.
- **ScopedImGuiStyle** — RAII push/pop of ImGui style/color overrides.
