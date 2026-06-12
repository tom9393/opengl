# Commands

Undo/redo system.

- **Command** — base interface (`Undo`/`Redo`). Commands resolve what they need (entities by UUID, the active scene) at Undo/Redo time rather than holding raw handles, so they stay valid across entity recreation.
- **CommandHistory** — owns the undo/redo stacks, depth capped at 100. `Execute` applies a freshly-built command and records it; `Push` records a command whose effect was already applied live (e.g. a finished ImGui drag). `CommandHistory::Global()` is the single shared history used for both scene and mesh edits.
- **EntitySnapshot** — subtree memento (components + children, UUIDs preserved) that the scene commands are built on.
- **SceneCommands** — `ModifyEntityCommand`, `CreateEntityCommand`, `DeleteEntityCommand`, `ReparentCommand`.
- **MaterialEditCommand** — undo for material property edits.
- **MeshEditCommand** — restores a whole `MeshEditSession::State` and re-syncs the render mesh; the global history is cleared when the mesh edit session ends.
