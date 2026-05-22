#pragma once

#include "pch.h"
#include "Commands/CommandHistory.h"
#include "Modeling/HalfEdgeMesh.h"
#include "Scene/Entity.h"

namespace Engine {

    // Bridges an entity's render Mesh and an editable HalfEdgeMesh.
    //
    // Begin clones the entity's mesh into a unique memory asset (so edits never touch a
    // shared or source asset), welds render vertices into topological vertices, and
    // builds the half-edge mesh. SyncToMesh regenerates the render mesh (triangulate +
    // recompute normals) and reuploads it under the clone's handle. End either keeps the
    // edited clone (commit) or restores the original mesh handle (discard).
    //
    // v1 behaviour: welding merges split vertices, so hard-edge normals come back
    // recomputed as smooth. UVs/tangents are carried from a representative source vertex
    // per welded vertex; geometry created by later edits gets defaults.
    class MeshEditSession {
    public:
        bool Begin(Entity entity);
        void SyncToMesh();
        void End(bool commit);

        // Topology edits on the active mesh (selected face indices). Each keeps the
        // carried attribute arrays in sync with the half-edge vertex set and re-uploads
        // via SyncToMesh. Returns false (mesh untouched) when inactive or when the
        // underlying half-edge op fails its consistency check.
        bool ExtrudeFaces(const std::vector<int>& faces);
        bool DeleteFaces(const std::vector<int>& faces);
        bool SubdivideFaces(const std::vector<int>& faces);

        bool IsActive() const { return m_Active; }
        Entity GetEntity() const { return m_Entity; }
        HalfEdgeMesh& Mesh() { return m_HalfEdge; }
        const HalfEdgeMesh& Mesh() const { return m_HalfEdge; }

        // Full editable geometry of the session: the half-edge mesh plus every parallel
        // attribute array. A topology/vertex/UV edit can be undone by restoring one of
        // these and re-syncing — see MeshEditCommand. These edits are recorded on the
        // global command history, which is cleared when the session ends.
        struct State {
            HalfEdgeMesh HalfEdge;
            std::vector<glm::vec2> UV;
            std::vector<glm::vec3> Tangent;
            std::vector<glm::vec3> Bitangent;
            std::vector<glm::vec2> UVVerts;
            std::vector<int> CornerUV;
        };

        State CaptureState() const;
        void RestoreState(const State& state);

        // Per-corner (face-loop) UV layer used by the UV editor. m_UVVerts holds the
        // movable UV points; CornerUV()[he] indexes into UVVerts for the corner at
        // half-edge he's origin within its face (-1 for boundary half-edges). Corners
        // that share a 3D vertex but differ in UV reference different UV verts, which is
        // how seams/islands are represented. After moving UV verts, call SyncToMesh to
        // push the change to the render mesh.
        std::vector<glm::vec2>& UVVerts() { return m_UVVerts; }
        const std::vector<glm::vec2>& UVVerts() const { return m_UVVerts; }
        const std::vector<int>& CornerUV() const { return m_CornerUV; }

    private:
        // Grows the carried attribute arrays by one entry per appended vertex, copied
        // from the given source vertex (used by extrude/subdivide).
        void AppendCarriedAttributes(const std::vector<int>& sources);

        // Build the per-corner UV layer from the imported render mesh: corners sharing a
        // welded vertex but differing in UV become separate UV verts (seams/islands).
        void BuildCornerUVs(const std::vector<std::vector<int>>& faces,
                            const std::vector<std::vector<glm::vec2>>& faceCornerUV);

        // After an append-only edit (extrude/subdivide), assign UV verts to the half-edges
        // appended past oldHalfEdgeCount, seeded from the per-vertex UVs.
        void SeedNewCornerUVs(int oldHalfEdgeCount);

        bool m_Active = false;
        Entity m_Entity;
        UUID m_OriginalMeshHandle = 0;
        UUID m_EditMeshHandle = 0;

        HalfEdgeMesh m_HalfEdge;

        // Carried per topological (welded) vertex; normals are recomputed each sync.
        // m_UV is the representative per-vertex UV, kept as the seed for corners created
        // by later topology edits; the UV editor edits the per-corner layer below.
        std::vector<glm::vec2> m_UV;
        std::vector<glm::vec3> m_Tangent;
        std::vector<glm::vec3> m_Bitangent;

        // Per-corner UV layer (see UVVerts/CornerUV accessors). m_CornerUV is indexed by
        // half-edge and stays the same length as the half-edge array.
        std::vector<glm::vec2> m_UVVerts;
        std::vector<int> m_CornerUV;
    };

}
