#include "pch.h"
#include "Modeling/HalfEdgeMesh.h"

namespace Engine {

    void HalfEdgeMesh::Clear()
    {
        m_Vertices.clear();
        m_HalfEdges.clear();
        m_Faces.clear();
    }

    bool HalfEdgeMesh::Build(const std::vector<glm::vec3>& positions, const std::vector<std::vector<int>>& faces)
    {
        Clear();

        for (const auto& face : faces) {
            if (face.size() < 3) {
                std::cout << "[HalfEdgeMesh] Rejected face with fewer than 3 vertices." << std::endl;
                return false;
            }

            for (int v : face) {
                if (v < 0 || v >= (int)positions.size()) {
                    std::cout << "[HalfEdgeMesh] Face references out-of-range vertex." << std::endl;
                    return false;
                }
            }
        }

        m_Vertices.resize(positions.size());
        for (size_t i = 0; i < positions.size(); ++i)
            m_Vertices[i].Position = positions[i];

        // (origin, dest) -> half-edge index. For twin matching, repeated directed edge means input is non-manifold or badly wound
        std::map<std::pair<int, int>, int> edgeMap;

        for (size_t f = 0; f < faces.size(); ++f) {
            const std::vector<int>& face = faces[f];
            int n = (int)face.size();
            int faceIndex = (int)m_Faces.size();
            int base = (int)m_HalfEdges.size();

            for (int k = 0; k < n; ++k) {
                int origin = face[k];
                int dest = face[(k + 1) % n];

                auto key = std::make_pair(origin, dest);
                if (edgeMap.count(key)) {
                    std::cout << "[HalfEdgeMesh] Non-manifold input: directed edge shared by two faces." << std::endl;
                    Clear();
                    return false;
                }

                int heIndex = base + k;
                HalfEdge he;
                he.Origin = origin;
                he.Face = faceIndex;
                he.Next = base + (k + 1) % n;
                m_HalfEdges.push_back(he);

                edgeMap[key] = heIndex;
                if (m_Vertices[origin].HalfEdge == InvalidIndex)
                    m_Vertices[origin].HalfEdge = heIndex;
            }

            m_Faces.push_back({ base });
        }

        // Match interior twins; the rest are edges on the boundary
        std::vector<int> boundaryInterior;
        for (int he = 0; he < (int)m_HalfEdges.size(); ++he) {
            int origin = m_HalfEdges[he].Origin;
            int dest = m_HalfEdges[m_HalfEdges[he].Next].Origin;
            auto it = edgeMap.find({ dest, origin });
            if (it != edgeMap.end())
                m_HalfEdges[he].Twin = it->second;
            else
                boundaryInterior.push_back(he);
        }

        // Create one boundary half-edge (Face == InvalidIndex) per boundary edge running opposite to its interior twin
        // A vertex with two outgoing boundary half-edges is a non-manifold vertex
        std::map<int, int> boundaryByOrigin;
        for (int interior : boundaryInterior) {
            int dest = m_HalfEdges[m_HalfEdges[interior].Next].Origin;

            int bIndex = (int)m_HalfEdges.size();
            HalfEdge b;
            b.Origin = dest;
            b.Face = InvalidIndex;
            b.Twin = interior;
            m_HalfEdges.push_back(b);
            m_HalfEdges[interior].Twin = bIndex;

            if (boundaryByOrigin.count(dest)) {
                std::cout << "[HalfEdgeMesh] Non-manifold input: vertex on multiple boundary loops." << std::endl;
                Clear();
                return false;
            }
            boundaryByOrigin[dest] = bIndex;
            m_Vertices[dest].HalfEdge = bIndex;
        }

        // Close boundary loops, the boundary half-edge after b starts where b ends
        for (auto& kv : boundaryByOrigin) {
            int bIndex = kv.second;
            int destOfB = m_HalfEdges[m_HalfEdges[bIndex].Twin].Origin;
            auto it = boundaryByOrigin.find(destOfB);
            if (it == boundaryByOrigin.end()) {
                std::cout << "[HalfEdgeMesh] Failed to close boundary loop (non-manifold)." << std::endl;
                Clear();
                return false;
            }
            m_HalfEdges[bIndex].Next = it->second;
        }

        return true;
    }

    bool HalfEdgeMesh::IsBoundaryEdge(int he) const
    {
        return m_HalfEdges[he].Face == InvalidIndex || m_HalfEdges[m_HalfEdges[he].Twin].Face == InvalidIndex;
    }

    bool HalfEdgeMesh::IsBoundaryVertex(int v) const
    {
        for (int he : OutgoingHalfEdges(v))
            if (IsBoundaryHalfEdge(he))
                return true;

        return false;
    }

    std::vector<int> HalfEdgeMesh::FaceHalfEdges(int face) const
    {
        std::vector<int> result;
        int start = m_Faces[face].HalfEdge;
        int he = start;
        do {
            result.push_back(he);
            he = m_HalfEdges[he].Next;
        } while (he != start);

        return result;
    }

    std::vector<int> HalfEdgeMesh::FaceVertices(int face) const
    {
        std::vector<int> result;
        for (int he : FaceHalfEdges(face))
            result.push_back(m_HalfEdges[he].Origin);
        return result;
    }

    std::vector<int> HalfEdgeMesh::OutgoingHalfEdges(int v) const
    {
        std::vector<int> result;
        int start = m_Vertices[v].HalfEdge;
        if (start == InvalidIndex)
            return result;
        int he = start;
        do {
            result.push_back(he);
            he = m_HalfEdges[m_HalfEdges[he].Twin].Next; // rotate around v
        } while (he != start);

        return result;
    }

    std::vector<int> HalfEdgeMesh::VertexNeighbors(int v) const
    {
        std::vector<int> result;
        for (int he : OutgoingHalfEdges(v))
            result.push_back(Destination(he));

        return result;
    }

    std::vector<std::pair<int, int>> HalfEdgeMesh::Edges() const
    {
        std::vector<std::pair<int, int>> result;
        for (int he = 0; he < (int)m_HalfEdges.size(); ++he) {
            int twin = m_HalfEdges[he].Twin;
            if (he < twin)
                result.emplace_back(m_HalfEdges[he].Origin, m_HalfEdges[twin].Origin);
        }

        return result;
    }

    glm::vec3 HalfEdgeMesh::FaceCentroid(int face) const
    {
        glm::vec3 centroid(0.0f);
        std::vector<int> verts = FaceVertices(face);
        if (verts.empty())
            return centroid;
        for (int v : verts)
            centroid += m_Vertices[v].Position;

        return centroid / (float)verts.size();
    }

    glm::vec3 HalfEdgeMesh::FaceNormal(int face) const
    {
        glm::vec3 normal(0.0f);
        std::vector<int> verts = FaceVertices(face);
        int count = (int)verts.size();
        for (int i = 0; i < count; ++i) {
            const glm::vec3& cur = m_Vertices[verts[i]].Position;
            const glm::vec3& nxt = m_Vertices[verts[(i + 1) % count]].Position;
            normal.x += (cur.y - nxt.y) * (cur.z + nxt.z);
            normal.y += (cur.z - nxt.z) * (cur.x + nxt.x);
            normal.z += (cur.x - nxt.x) * (cur.y + nxt.y);
        }

        float len = glm::length(normal);
        return len > 0.0f ? normal / len : normal;
    }

    bool HalfEdgeMesh::CheckConsistency() const
    {
        int n = (int)m_HalfEdges.size();
        for (int he = 0; he < n; ++he) {
            const HalfEdge& h = m_HalfEdges[he];
            if (h.Twin < 0 || h.Twin >= n || h.Twin == he || m_HalfEdges[h.Twin].Twin != he) {
                std::cout << "[HalfEdgeMesh] Consistency: bad twin at half-edge " << he << std::endl;
                return false;
            }
            if (h.Next < 0 || h.Next >= n) {
                std::cout << "[HalfEdgeMesh] Consistency: bad next at half-edge " << he << std::endl;
                return false;
            }
            if (m_HalfEdges[h.Next].Face != h.Face) {
                std::cout << "[HalfEdgeMesh] Consistency: next crosses face at half-edge " << he << std::endl;
                return false;
            }
            if (h.Origin < 0 || h.Origin >= (int)m_Vertices.size()) {
                std::cout << "[HalfEdgeMesh] Consistency: bad origin at half-edge " << he << std::endl;
                return false;
            }
            int dest = m_HalfEdges[h.Twin].Origin;
            if (m_HalfEdges[h.Next].Origin != dest) {
                std::cout << "[HalfEdgeMesh] Consistency: next origin != dest at half-edge " << he << std::endl;
                return false;
            }
        }

        for (int f = 0; f < (int)m_Faces.size(); ++f) {
            int he = m_Faces[f].HalfEdge;
            if (he < 0 || he >= n || m_HalfEdges[he].Face != f) {
                std::cout << "[HalfEdgeMesh] Consistency: bad face reference at face " << f << std::endl;
                return false;
            }
        }

        return true;
    }

    void HalfEdgeMesh::RebuildVertexHalfEdges()
    {
        for (Vertex& v : m_Vertices)
            v.HalfEdge = InvalidIndex;

        for (int he = 0; he < (int)m_HalfEdges.size(); ++he) {
            int o = m_HalfEdges[he].Origin;
            if (m_Vertices[o].HalfEdge == InvalidIndex)
                m_Vertices[o].HalfEdge = he;
            else if (m_HalfEdges[he].Face == InvalidIndex) // prefer a boundary half-edge
                m_Vertices[o].HalfEdge = he;
        }
    }

    bool HalfEdgeMesh::RelinkBoundaryLoops()
    {
        std::map<int, int> boundaryByOrigin;
        for (int he = 0; he < (int)m_HalfEdges.size(); ++he) {
            if (m_HalfEdges[he].Face != InvalidIndex)
                continue;
            int o = m_HalfEdges[he].Origin;
            if (boundaryByOrigin.count(o)) // two boundary loops meet at a vertex
                return false;
            boundaryByOrigin[o] = he;
        }

        for (auto& kv : boundaryByOrigin)
        {
            int b = kv.second;
            int dest = m_HalfEdges[m_HalfEdges[b].Twin].Origin;
            auto it = boundaryByOrigin.find(dest);
            if (it == boundaryByOrigin.end())
                return false;
            m_HalfEdges[b].Next = it->second;
        }

        return true;
    }

    bool HalfEdgeMesh::ExtrudeFaces(const std::vector<int>& faces, std::vector<int>& newVertexSources)
    {
        newVertexSources.clear();
        if (faces.empty())
            return false;

        std::vector<bool> selFace(m_Faces.size(), false);
        for (int f : faces) {
            if (f < 0 || f >= (int)m_Faces.size())
                return false;
            selFace[f] = true;
        }

        // Region-boundary half-edges: a selected face's half-edge whose twin is not in the region
        // Capture each one's original endpoints + twin before any rewiring
        struct Bhe { int he, oldA, oldB, twin; };
        std::vector<Bhe> boundary;
        for (int he = 0; he < (int)m_HalfEdges.size(); ++he) {
            int f = m_HalfEdges[he].Face;
            if (f == InvalidIndex || !selFace[f])
                continue;
            int twin = m_HalfEdges[he].Twin;
            int tf = m_HalfEdges[twin].Face;
            if (tf != InvalidIndex && selFace[tf])
                continue; // interior to the region
            boundary.push_back({ he, m_HalfEdges[he].Origin, m_HalfEdges[twin].Origin, twin });
        }
        if (boundary.empty()) // e.g. an entire closed mesh selected: nothing to extrude against
            return false;

        std::vector<Vertex>   snapV = m_Vertices;
        std::vector<HalfEdge> snapH = m_HalfEdges;
        std::vector<Face>     snapF = m_Faces;
        auto revert = [&](const char* why) {
            m_Vertices = snapV; m_HalfEdges = snapH; m_Faces = snapF;
            newVertexSources.clear();
            std::cout << "[HalfEdgeMesh] Extrude failed: " << why << "; reverted." << std::endl;
        };

        // Duplicate each region-boundary vertex once
        std::map<int, int> dup;
        auto duplicate = [&](int v) {
            if (dup.count(v))
                return;
            Vertex vert;
            vert.Position = m_Vertices[v].Position;
            int nv = (int)m_Vertices.size();
            m_Vertices.push_back(vert);
            dup[v] = nv;
            newVertexSources.push_back(v);
        };
        for (const Bhe& b : boundary) { duplicate(b.oldA); duplicate(b.oldB); }

        // Lift the selected faces onto the duplicates: a selected-face half-edge whose origin was duplicated now starts at the duplicate
        for (int he = 0; he < (int)m_HalfEdges.size(); ++he) {
            int f = m_HalfEdges[he].Face;
            if (f == InvalidIndex || !selFace[f])
                continue;
            auto it = dup.find(m_HalfEdges[he].Origin);
            if (it != dup.end())
                m_HalfEdges[he].Origin = it->second;
        }

        // Build a side-wall quad per region-boundary edge and re-twin
        // Verticals (old->dup) are matched between adjacent walls through this map
        std::map<std::pair<int, int>, int> verticalMap;
        for (const Bhe& b : boundary) {
            int oldA = b.oldA, oldB = b.oldB;
            int na = dup[oldA], nb = dup[oldB];
            int t = b.twin; // non-selected side, still oldB->oldA
            int he = b.he;  // selected-face he, now na->nb after the lift

            int base = (int)m_HalfEdges.size();
            int e1 = base, e2 = base + 1, e3 = base + 2, e4 = base + 3;
            int fIdx = (int)m_Faces.size();

            HalfEdge h1; h1.Origin = oldA; h1.Face = fIdx; h1.Next = e2; m_HalfEdges.push_back(h1);
            HalfEdge h2; h2.Origin = oldB; h2.Face = fIdx; h2.Next = e3; m_HalfEdges.push_back(h2);
            HalfEdge h3; h3.Origin = nb;   h3.Face = fIdx; h3.Next = e4; m_HalfEdges.push_back(h3);
            HalfEdge h4; h4.Origin = na;   h4.Face = fIdx; h4.Next = e1; m_HalfEdges.push_back(h4);
            m_Faces.push_back({ e1 });

            // e1 (oldA->oldB) takes over the old twin t; e3 (nb->na) twins the lifted he
            m_HalfEdges[e1].Twin = t;  m_HalfEdges[t].Twin = e1;
            m_HalfEdges[e3].Twin = he; m_HalfEdges[he].Twin = e3;

            verticalMap[{ oldB, nb }] = e2; // e2: oldB->nb
            verticalMap[{ na, oldA }] = e4; // e4: na->oldA
        }

        // Match vertical twins between adjacent walls (reciprocal direction)
        for (auto& kv : verticalMap) {
            auto it = verticalMap.find({ kv.first.second, kv.first.first });
            if (it == verticalMap.end()) {
                revert("open region boundary");
                return false;
            }
            m_HalfEdges[kv.second].Twin = it->second;
        }

        RebuildVertexHalfEdges();

        if (!CheckConsistency()) {
            revert("invalid result");
            return false;
        }
        return true;
    }

    bool HalfEdgeMesh::DeleteFaces(const std::vector<int>& faces, std::vector<int>& vertexRemap, std::vector<int>& halfEdgeRemap)
    {
        vertexRemap.clear();
        halfEdgeRemap.clear();
        if (faces.empty())
            return false;

        std::vector<bool> selFace(m_Faces.size(), false);
        for (int f : faces) {
            if (f < 0 || f >= (int)m_Faces.size())
                return false;
            selFace[f] = true;
        }

        std::vector<Vertex>   snapV = m_Vertices;
        std::vector<HalfEdge> snapH = m_HalfEdges;
        std::vector<Face>     snapF = m_Faces;
        auto revert = [&](const char* why) {
            m_Vertices = snapV; m_HalfEdges = snapH; m_Faces = snapF;
            vertexRemap.clear();
            halfEdgeRemap.clear();
            std::cout << "[HalfEdgeMesh] Delete failed: " << why << "; reverted." << std::endl;
        };

        int heCount = (int)m_HalfEdges.size();
        std::vector<bool> removeHE(heCount, false);

        // Classify each half-edge of a deleted face
        for (int he = 0; he < heCount; ++he) {
            int f = m_HalfEdges[he].Face;
            if (f == InvalidIndex || !selFace[f])
                continue;
            int twin = m_HalfEdges[he].Twin;
            int tf = m_HalfEdges[twin].Face;
            if (tf == InvalidIndex) {
                // Edge sat on the mesh boundary: the whole edge disappears
                removeHE[he] = true;
                removeHE[twin] = true;
            } else if (selFace[tf]) {
                // Interior edge of the deleted region: disappears with both faces
                removeHE[he] = true;
            } else {
                // Twin survives: repurpose this half-edge as the new boundary edge
                m_HalfEdges[he].Face = InvalidIndex;
            }
        }

        // A vertex survives iff some kept half-edge starts there
        std::vector<bool> vAlive(m_Vertices.size(), false);
        for (int he = 0; he < heCount; ++he)
            if (!removeHE[he])
                vAlive[m_HalfEdges[he].Origin] = true;

        vertexRemap.assign(m_Vertices.size(), -1);
        int newV = 0;
        for (int v = 0; v < (int)m_Vertices.size(); ++v)
            if (vAlive[v]) vertexRemap[v] = newV++;

        std::vector<int> heRemap(heCount, -1);
        int newH = 0;
        for (int he = 0; he < heCount; ++he)
            if (!removeHE[he]) heRemap[he] = newH++;
        halfEdgeRemap = heRemap;

        std::vector<int> fRemap(m_Faces.size(), -1);
        int newF = 0;
        for (int f = 0; f < (int)m_Faces.size(); ++f)
            if (!selFace[f]) fRemap[f] = newF++;

        std::vector<Vertex> nv(newV);
        for (int v = 0; v < (int)m_Vertices.size(); ++v)
            if (vertexRemap[v] >= 0) nv[vertexRemap[v]] = m_Vertices[v];

        std::vector<HalfEdge> nh(newH);
        for (int he = 0; he < heCount; ++he) {
            if (heRemap[he] < 0)
                continue;
            HalfEdge h = m_HalfEdges[he];
            h.Origin = vertexRemap[h.Origin];
            h.Twin   = heRemap[h.Twin];
            // A repurposed boundary half-edge's Next may now point at a removed half-edge;
            // RelinkBoundaryLoops re-derives all boundary Next links below
            h.Next   = (h.Next >= 0 && heRemap[h.Next] >= 0) ? heRemap[h.Next] : InvalidIndex;
            h.Face   = (h.Face == InvalidIndex) ? InvalidIndex : fRemap[h.Face];
            nh[heRemap[he]] = h;
        }

        std::vector<Face> nf(newF);
        for (int f = 0; f < (int)m_Faces.size(); ++f) {
            if (fRemap[f] < 0)
                continue;
            nf[fRemap[f]].HalfEdge = heRemap[m_Faces[f].HalfEdge];
        }

        m_Vertices  = std::move(nv);
        m_HalfEdges = std::move(nh);
        m_Faces     = std::move(nf);

        if (!RelinkBoundaryLoops()) {
            revert("non-manifold boundary");
            return false;
        }
        RebuildVertexHalfEdges();

        if (!CheckConsistency()) {
            revert("invalid result");
            return false;
        }
        return true;
    }

    bool HalfEdgeMesh::SubdivideFaces(const std::vector<int>& faces, std::vector<int>& newVertexSources)
    {
        newVertexSources.clear();
        if (faces.empty())
            return false;

        std::vector<bool> selFace(m_Faces.size(), false);
        for (int f : faces) {
            if (f < 0 || f >= (int)m_Faces.size())
                return false;
            selFace[f] = true;
        }

        std::vector<Vertex>   snapV = m_Vertices;
        std::vector<HalfEdge> snapH = m_HalfEdges;
        std::vector<Face>     snapF = m_Faces;
        auto revert = [&](const char* why) {
            m_Vertices = snapV; m_HalfEdges = snapH; m_Faces = snapF;
            newVertexSources.clear();
            std::cout << "[HalfEdgeMesh] Subdivide failed: " << why << "; reverted." << std::endl;
        };

        // Split every edge incident to a selected face at its midpoint
        int origHE = (int)m_HalfEdges.size();
        std::vector<bool> visited(origHE, false);
        for (int he = 0; he < origHE; ++he) {
            if (visited[he])
                continue;
            int tw = m_HalfEdges[he].Twin;
            visited[he] = true;
            if (tw < origHE) visited[tw] = true;

            int f1 = m_HalfEdges[he].Face;
            int f2 = m_HalfEdges[tw].Face;
            bool incident = (f1 != InvalidIndex && selFace[f1]) || (f2 != InvalidIndex && selFace[f2]);
            if (!incident)
                continue;

            int a = m_HalfEdges[he].Origin;
            int b = m_HalfEdges[tw].Origin;
            glm::vec3 pa = m_Vertices[a].Position;
            glm::vec3 pb = m_Vertices[b].Position;
            int heNext = m_HalfEdges[he].Next, twNext = m_HalfEdges[tw].Next;
            int heFace = m_HalfEdges[he].Face, twFace = m_HalfEdges[tw].Face;

            Vertex mid; mid.Position = 0.5f * (pa + pb);
            int m = (int)m_Vertices.size();
            m_Vertices.push_back(mid);
            newVertexSources.push_back(a);

            int h2 = (int)m_HalfEdges.size();   // m->b, in he's face
            int tw2 = h2 + 1;                   // m->a, in tw's face
            HalfEdge H2; H2.Origin = m; H2.Face = heFace; H2.Next = heNext; H2.Twin = tw;
            HalfEdge TW2; TW2.Origin = m; TW2.Face = twFace; TW2.Next = twNext; TW2.Twin = he;
            m_HalfEdges.push_back(H2);
            m_HalfEdges.push_back(TW2);

            m_HalfEdges[he].Next = h2;  m_HalfEdges[he].Twin = tw2; // he is now a->m
            m_HalfEdges[tw].Next = tw2; m_HalfEdges[tw].Twin = h2;  // tw is now b->m
        }

        // Replace each selected face with quads around a new centre
        for (int f = 0; f < (int)selFace.size(); ++f) {
            if (!selFace[f])
                continue;

            std::vector<int> loop = FaceHalfEdges(f); // alternates corner-origin / mid-origin
            int total = (int)loop.size();
            if (total < 6 || (total % 2) != 0) { // need >=3 corners, strict alternation
                revert("unexpected face loop");
                return false;
            }
            int n = total / 2;

            // Centre = average of the corner positions (even loop slots are corner-origins)
            glm::vec3 centroid(0.0f);
            for (int i = 0; i < n; ++i)
                centroid += m_Vertices[m_HalfEdges[loop[2 * i]].Origin].Position;
            centroid /= (float)n;

            int firstCorner = m_HalfEdges[loop[0]].Origin;
            int c = (int)m_Vertices.size();
            Vertex centre; centre.Position = centroid;
            m_Vertices.push_back(centre);
            newVertexSources.push_back(firstCorner);

            // Spokes (mid<->centre) matched within this face via reciprocal direction
            std::map<std::pair<int, int>, int> spokeMap;
            for (int i = 0; i < n; ++i) {
                int out = loop[2 * i];                  // c_i -> m_i (existing)
                int in  = loop[(2 * i + total - 1) % total]; // m_{i-1} -> c_i (existing)
                int mi = m_HalfEdges[m_HalfEdges[out].Twin].Origin; // Dest(out)
                int mPrev = m_HalfEdges[in].Origin;     // m_{i-1}

                int faceIdx = (i == 0) ? f : (int)m_Faces.size();
                int sA = (int)m_HalfEdges.size(); // m_i -> centre
                int sB = sA + 1;                  // centre -> m_{i-1}
                HalfEdge ha; ha.Origin = mi; ha.Face = faceIdx; ha.Next = sB;
                HalfEdge hb; hb.Origin = c;  hb.Face = faceIdx; hb.Next = in;
                m_HalfEdges.push_back(ha);
                m_HalfEdges.push_back(hb);

                m_HalfEdges[out].Face = faceIdx; m_HalfEdges[out].Next = sA;
                m_HalfEdges[in].Face  = faceIdx; m_HalfEdges[in].Next  = out;

                if (i == 0) m_Faces[f].HalfEdge = out;
                else        m_Faces.push_back({ out });

                spokeMap[{ mi, c }] = sA;
                spokeMap[{ c, mPrev }] = sB;
            }

            for (auto& kv : spokeMap) {
                auto it = spokeMap.find({ kv.first.second, kv.first.first });
                if (it == spokeMap.end()) {
                    revert("spoke mismatch");
                    return false;
                }
                m_HalfEdges[kv.second].Twin = it->second;
            }
        }

        RebuildVertexHalfEdges();

        if (!CheckConsistency()) {
            revert("invalid result");
            return false;
        }
        return true;
    }

}
