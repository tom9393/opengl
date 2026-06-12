# Panels/Viewport

The 3D viewport and its editing modes.

- **ViewportPanel** — hosts the mode tabs (`ViewportTab`) and switches between the three viewports.
- **ViewportPanelBase** — shared base: presents the rendered image, handles resize/focus, camera input, and mouse picking. All modes share a **ViewportContext** (scene, renderer, editor camera).
- **ObjectViewportPanel** — scene/object editing: selection, ImGuizmo transform gizmos, drag-and-drop of assets into the scene.
- **ModelingViewportPanel** — mesh editing mode; drives `Modeling/MeshEditSession` topology edits (extrude, delete, subdivide) with element selection.
- **UVViewportPanel** — 2D UV editor over the session's per-corner UV layer (seams/islands).
