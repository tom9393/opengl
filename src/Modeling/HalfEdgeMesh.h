#pragma once

#include "pch.h"

namespace Engine {

    // Half-edge (DCEL) mesh used for topology editing. Supports n-gon faces.
    // Scope: manifold + boundary surfaces only.
    //
    // Boundary edges are represented explicitly: each boundary edge has a half-edge
    // whose Face == InvalidIndex, and these boundary half-edges are linked through
    // Next into boundary loops. This keeps traversal uniform — rotating around a
    // vertex via Next(Twin(he)) walks the full one-ring, crossing boundaries cleanly.
    // Every half-edge therefore has a valid Twin and Next.
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

        // Build from welded positions and polygonal faces (each face a CCW list of
        // vertex indices). Returns false and leaves the mesh empty if the input is
        // degenerate or non-manifold.
        bool Build(const std::vector<glm::vec3>& positions,
                   const std::vector<std::vector<int>>& faces);

        void Clear();
        bool IsEmpty() const { return m_Vertices.empty(); }

        const std::vector<Vertex>&   Vertices()  const { return m_Vertices; }
        const std::vector<HalfEdge>& HalfEdges() const { return m_HalfEdges; }
        const std::vector<Face>&     Faces()     const { return m_Faces; }

        const glm::vec3& Position(int v) const { return m_Vertices[v].Position; }
        glm::vec3&       Position(int v)       { return m_Vertices[v].Position; }

        // Destination vertex of a half-edge.
        int Destination(int he) const { return m_HalfEdges[m_HalfEdges[he].Twin].Origin; }

        bool IsBoundaryHalfEdge(int he) const { return m_HalfEdges[he].Face == InvalidIndex; }
        bool IsBoundaryEdge(int he) const;
        bool IsBoundaryVertex(int v) const;

        // Vertices / half-edges of a face, in winding order.
        std::vector<int> FaceVertices(int face) const;
        std::vector<int> FaceHalfEdges(int face) const;

        // One-ring around a vertex: half-edges leaving it, and the neighbor vertices.
        std::vector<int> OutgoingHalfEdges(int v) const;
        std::vector<int> VertexNeighbors(int v) const;

        // Each undirected edge once, as an (origin, destination) vertex pair.
        std::vector<std::pair<int, int>> Edges() const;

        glm::vec3 FaceCentroid(int face) const;
        glm::vec3 FaceNormal(int face) const; // Newell's method (robust for n-gons)

        // --- Topology edits (in-place half-edge surgery) ---
        //
        // Each op snapshots the DCEL, splices it directly, then re-validates manifold
        // invariants; if the result is broken the snapshot is restored and the op returns
        // false (the mesh is left untouched). The caller keeps any parallel per-vertex
        // data (UVs/tangents) in sync via the out-params below.

        // Extrude the given faces as one connected region: duplicates the region's
        // boundary vertices, inserts side-wall quads, and re-points the selected faces
        // onto the duplicates so they can be moved. Vertices are only appended;
        // newVertexSources[i] is the source vertex that appended vertex (oldCount + i)
        // copies attributes from. Faces keep their indices (walls are appended).
        // Requires simple (non-pinched) region boundary loops.
        bool ExtrudeFaces(const std::vector<int>& faces, std::vector<int>& newVertexSources);

        // Delete the given faces, exposing new boundary loops and dropping vertices that
        // become orphaned. Compacts all arrays; vertexRemap (size = old vertex count)
        // maps each old vertex to its new index, or -1 if removed. halfEdgeRemap
        // (size = old half-edge count) maps each old half-edge to its new index, or -1 if
        // removed, so callers can compact parallel per-half-edge data (e.g. corner UVs).
        bool DeleteFaces(const std::vector<int>& faces, std::vector<int>& vertexRemap,
                         std::vector<int>& halfEdgeRemap);

        // Subdivide the given faces: splits every incident edge at its midpoint and adds
        // a center vertex per selected face, replacing each selected face with quads.
        // Edge midpoints are shared, so neighbouring unselected faces stay watertight
        // (they absorb the midpoint, becoming n-gons). Vertices are only appended;
        // newVertexSources[i] as in ExtrudeFaces. Selected faces are replaced (indices
        // change), so callers should clear face selection afterwards.
        bool SubdivideFaces(const std::vector<int>& faces, std::vector<int>& newVertexSources);

    private:
        // Validates the core DCEL invariants (reciprocal twins, closed Next loops,
        // one outgoing boundary half-edge per boundary vertex). Logs and returns false
        // on the first broken invariant. Used as the post-edit safety check.
        bool CheckConsistency() const;

        // Rebuilds m_Vertices[v].HalfEdge for every vertex, preferring an outgoing
        // boundary half-edge where one exists (so OutgoingHalfEdges spans the full fan).
        void RebuildVertexHalfEdges();

        // Re-derives boundary half-edge Next links into closed loops from scratch.
        // Returns false if a vertex has more than one outgoing boundary half-edge
        // (a non-manifold result).
        bool RelinkBoundaryLoops();

        std::vector<Vertex>   m_Vertices;
        std::vector<HalfEdge> m_HalfEdges;
        std::vector<Face>     m_Faces;
    };

}
