# Panels/SceneHierarchy

Entity tree and inspector.

- **SceneHierarchyPanel** — the entity tree: selection, drag-and-drop reparenting, create/delete/duplicate, and the inspector for the selected entity. Edits go through `Commands/` so they are undoable.
- **DrawComponent** — shared scaffolding for inspector sections (collapsing header, remove button).
- **Draw\*** (one header per component) — the per-component inspectors: `DrawTransform`, `DrawMesh`, `DrawMaterial`, `DrawCamera`, `DrawLight`, `DrawRigidBody`, `DrawParticleEmitter`, `DrawLua`.
- **DrawVec3** — the labeled X/Y/Z drag widget used by the inspectors.
