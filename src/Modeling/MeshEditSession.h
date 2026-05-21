#pragma once

#include "pch.h"
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

    private:
        // Grows the carried attribute arrays by one entry per appended vertex, copied
        // from the given source vertex (used by extrude/subdivide).
        void AppendCarriedAttributes(const std::vector<int>& sources);

        bool m_Active = false;
        Entity m_Entity;
        UUID m_OriginalMeshHandle = 0;
        UUID m_EditMeshHandle = 0;

        HalfEdgeMesh m_HalfEdge;

        // Carried per topological (welded) vertex; normals are recomputed each sync.
        std::vector<glm::vec2> m_UV;
        std::vector<glm::vec3> m_Tangent;
        std::vector<glm::vec3> m_Bitangent;
    };

}
