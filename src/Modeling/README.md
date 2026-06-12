# Modeling

In-editor mesh editing on a half-edge (DCEL) structure.

- **HalfEdgeMesh** — DCEL supporting n-gon faces over manifold + boundary surfaces. Topology ops (`ExtrudeFaces`, `DeleteFaces`, `SubdivideFaces`) work in place: they snapshot the DCEL, splice it directly, then re-validate manifold invariants — restoring the snapshot if the result is broken.
- **MeshEditSession** — bridges an entity's render `Mesh` and an editable `HalfEdgeMesh`:
  - `Begin` clones the mesh into a unique in-memory asset (edits never touch a shared/source asset) and welds render vertices into topological vertices.
  - `SyncToMesh` triangulates, recomputes normals, and re-uploads to the GPU.
  - `End` commits the clone or restores the original handle.

  The session carries parallel per-vertex attributes (UV/tangent) plus a per-corner UV layer (`UVVerts`/`CornerUV`) representing seams and islands.

`Panels/Viewport/ModelingViewportPanel` drives the topology edits; `UVViewportPanel` edits the per-corner UV layer. Undo runs through `Commands/MeshEditCommand`, which restores whole session states.
