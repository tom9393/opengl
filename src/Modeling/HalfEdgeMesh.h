#pragma once

#include "pch.h"

namespace Engine {

    // Half-edge (DCEL) mesh used for topology editing. Supports n-gon faces.
    class HalfEdgeMesh {
    public:
        static constexpr int InvalidIndex = -1;

        struct Vertex {
            glm::vec3 Position{0.0f};
            int HalfEdge = InvalidIndex; // one half-edge with Origin == this vertex
        };

        struct HalfEdge {
            int Origin = InvalidIndex; // vertex this half-edge starts at
            int Twin   = InvalidIndex; // opposite half-edge (other side of the edge)
            int Next   = InvalidIndex; // next half-edge around the same face/boundary loop
            int Face   = InvalidIndex; // owning face, or InvalidIndex on the boundary
        };

        struct Face {
            int HalfEdge = InvalidIndex; // one half-edge belonging to this face
        };

        // Returns false if mesh empty or if input is bad/non-manifold
        bool Build(const std::vector<glm::vec3>& positions, const std::vector<std::vector<int>>& faces);

        void Clear();
        bool IsEmpty() const { return m_Vertices.empty(); }

        const std::vector<Vertex>&   Vertices()  const { return m_Vertices; }
        const std::vector<HalfEdge>& HalfEdges() const { return m_HalfEdges; }
        const std::vector<Face>&     Faces()     const { return m_Faces; }

        const glm::vec3& Position(int v) const { return m_Vertices[v].Position; }
        glm::vec3&       Position(int v)       { return m_Vertices[v].Position; }

        // Destination vertex of a half-edge
        int Destination(int he) const { return m_HalfEdges[m_HalfEdges[he].Twin].Origin; }

        bool IsBoundaryHalfEdge(int he) const { return m_HalfEdges[he].Face == InvalidIndex; }
        bool IsBoundaryEdge(int he) const;
        bool IsBoundaryVertex(int v) const;

        // Vertices / half-edges of a face, in winding order
        std::vector<int> FaceVertices(int face) const;
        std::vector<int> FaceHalfEdges(int face) const;

        // One-ring around a vertex: half-edges leaving it, and the neighbor vertices
        std::vector<int> OutgoingHalfEdges(int v) const;
        std::vector<int> VertexNeighbors(int v) const;

        // Each undirected edge once, as an (origin, destination) vertex pair
        std::vector<std::pair<int, int>> Edges() const;

        glm::vec3 FaceCentroid(int face) const;
        glm::vec3 FaceNormal(int face) const; // Newell's method (robust for n-gons)

        bool ExtrudeFaces(const std::vector<int>& faces, std::vector<int>& newVertexSources);
        bool DeleteFaces(const std::vector<int>& faces, std::vector<int>& vertexRemap, std::vector<int>& halfEdgeRemap);
        bool SubdivideFaces(const std::vector<int>& faces, std::vector<int>& newVertexSources);

    private:
        bool CheckConsistency() const;
        void RebuildVertexHalfEdges();
        bool RelinkBoundaryLoops();

        std::vector<Vertex>   m_Vertices;
        std::vector<HalfEdge> m_HalfEdges;
        std::vector<Face>     m_Faces;
    };

}
