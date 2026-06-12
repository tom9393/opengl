# Panels

The editor UI. `EditorLayer` composes the editor from these ImGui panels.

## Subfolders

- [Viewport/](Viewport/) — the 3D viewport and its object/modeling/UV editing modes.
- [SceneHierarchy/](SceneHierarchy/) — entity tree plus per-component inspectors.
- [ContentBrowser/](ContentBrowser/) — asset browser with async thumbnails.

## Standalone panels

- **MenubarPanel** — top menu bar (file open/save, scene operations).
- **ToolbarPanel** — play/stop and gizmo operation controls.
- **StatsPanel** — frame stats and per-pass GPU timings from the render graph.
- **SettingsPanel** — scene settings (environment, post-process, …).
- **ShaderPanel** — shader inspection/reload.
- **PassPanel** — selects which render-graph output the viewport displays.
