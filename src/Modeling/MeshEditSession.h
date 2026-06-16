#pragma once

#include "pch.h"
#include "Commands/CommandHistory.h"
#include "Modeling/HalfEdgeMesh.h"
#include "Scene/Entity.h"

namespace Engine {

    // Bridges an entity's render Mesh and an editable HalfEdgeMesh
    class MeshEditSession {
    public:
        bool Begin(Entity entity);
        void SyncToMesh();
        void End(bool commit);

        bool ExtrudeFaces(const std::vector<int>& faces);
        bool DeleteFaces(const std::vector<int>& faces);
        bool SubdivideFaces(const std::vector<int>& faces);

        bool IsActive() const { return m_Active; }
        Entity GetEntity() const { return m_Entity; }
        HalfEdgeMesh& Mesh() { return m_HalfEdge; }
        const HalfEdgeMesh& Mesh() const { return m_HalfEdge; }

        // Half-edge mesh plus every parallel attribute. Used to track state for redo/undo
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

        std::vector<glm::vec2>& UVVerts() { return m_UVVerts; }
        const std::vector<glm::vec2>& UVVerts() const { return m_UVVerts; }
        const std::vector<int>& CornerUV() const { return m_CornerUV; }

    private:
        // Grows the carried attribute arrays by one entry per appended vertex
        void AppendCarriedAttributes(const std::vector<int>& sources);

        // Build the per-corner UV layer from the imported render mesh
        // Corner shares a welded vertex but differing in UV become separate UV verts (seams/islands)
        void BuildCornerUVs(const std::vector<std::vector<int>>& faces, const std::vector<std::vector<glm::vec2>>& faceCornerUV);

        // After an append-only edit (extrude/subdivide), assign UV verts to the half-edges
        // Appended past oldHalfEdgeCount, seeded from the per-vertex UVs
        void SeedNewCornerUVs(int oldHalfEdgeCount);

        bool m_Active = false;
        Entity m_Entity;
        UUID m_OriginalMeshHandle = 0;
        UUID m_EditMeshHandle = 0;

        HalfEdgeMesh m_HalfEdge;

        // Normals are recomputed each sync
        std::vector<glm::vec2> m_UV;
        std::vector<glm::vec3> m_Tangent;
        std::vector<glm::vec3> m_Bitangent;

        // m_CornerUV indexed by half-edge and stays the same length as the half-edge array
        std::vector<glm::vec2> m_UVVerts;
        std::vector<int> m_CornerUV;
    };

}
