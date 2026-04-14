#include "PrimitiveBuilder.h"
#include <vector>
#include <cmath>
#include <algorithm>

namespace Engine {

    // Helper to streamline creating and registering the mesh
    static UUID RegisterMesh(const std::string& name, const std::vector<float>& vertices, const std::vector<uint32_t>& indices, bool hasNormals = false, bool hasTangents = false, bool hasBitangents = false)
    {
        Shared<VertexArray> vao = VertexArray::Create();
        Shared<VertexBuffer> vbo = VertexBuffer::Create((float*)vertices.data(), vertices.size() * sizeof(float));
        
        // Dynamically assign layout based on what was calculated
        if (hasNormals && hasTangents && hasBitangents) {
            vbo->SetLayout({
                { ShaderDataType::Float3, "a_Position" },
                { ShaderDataType::Float3, "a_Normal" },
                { ShaderDataType::Float2, "a_TexCoord" },
                { ShaderDataType::Float3, "a_Tangent" },
                { ShaderDataType::Float3, "a_Bitangent" }
            });
        } else if (hasNormals && hasTangents) {
            vbo->SetLayout({
                { ShaderDataType::Float3, "a_Position" },
                { ShaderDataType::Float3, "a_Normal" },
                { ShaderDataType::Float2, "a_TexCoord" },
                { ShaderDataType::Float3, "a_Tangent" }
            });
        } else if (hasNormals) {
            vbo->SetLayout({
                { ShaderDataType::Float3, "a_Position" },
                { ShaderDataType::Float3, "a_Normal" },
                { ShaderDataType::Float2, "a_TexCoord" }
            });
        } else {
            vbo->SetLayout({
                { ShaderDataType::Float3, "a_Position" },
                { ShaderDataType::Float2, "a_TexCoord" }
            });
        }
        
        vao->AddVertexBuffer(vbo);

        Shared<IndexBuffer> ebo = IndexBuffer::Create((unsigned int*)indices.data(), indices.size());
        vao->SetIndexBuffer(ebo);

        std::vector<Shared<VertexBuffer>> vbos = { vbo };
        Shared<Mesh> mesh = MakeShared<Mesh>(vertices, indices, vao, vbos, ebo); 

        glm::vec3 minAABB(std::numeric_limits<float>::max());
        glm::vec3 maxAABB(std::numeric_limits<float>::lowest());

        // Dynamically calculate the float stride
        size_t vertexStride = 3 + (hasNormals ? 3 : 0) + 2 + (hasTangents ? 3 : 0) + (hasBitangents ? 3 : 0);

        for (size_t i = 0; i < vertices.size(); i += vertexStride)
        {
            float x = vertices[i];
            float y = vertices[i + 1];
            float z = vertices[i + 2];

            if (x < minAABB.x) minAABB.x = x;
            if (y < minAABB.y) minAABB.y = y;
            if (z < minAABB.z) minAABB.z = z;

            if (x > maxAABB.x) maxAABB.x = x;
            if (y > maxAABB.y) maxAABB.y = y;
            if (z > maxAABB.z) maxAABB.z = z;
        }

        AABB aabb;
        aabb.Min = minAABB; 
        aabb.Max = maxAABB;
        mesh->SetAABB(aabb);

        return AssetManager::CreateMemoryAsset(AssetType::Mesh, name, mesh);
    }

    UUID PrimitiveBuilder::CreatePrimitive(PrimitiveSpecification spec, const std::string& name)
    {
        if (m_ReadyTypes.find(spec.Type) != m_ReadyTypes.end())
        {
            return m_ReadyTypes[spec.Type];
        };

        std::vector<float> vertices;
        std::vector<uint32_t> indices;

        // Route to the correct generator
        switch (spec.Type) {
            case PrimitiveType::Plane:    BuildPlane(spec, vertices, indices); break;
            case PrimitiveType::Cube:     BuildCube(spec, vertices, indices); break;
            case PrimitiveType::Sphere:   BuildSphere(spec, vertices, indices); break;
            case PrimitiveType::Cylinder: BuildCylinder(spec, vertices, indices); break;
            case PrimitiveType::Capsule:  BuildCapsule(spec, vertices, indices); break;
        }

        UUID registeredMesh =  RegisterMesh(name, vertices, indices, spec.CalculateNormals, spec.CalculateTangents, spec.CalculateBitangents);
        m_ReadyTypes[spec.Type] = registeredMesh;

        return registeredMesh;
    }

    void PrimitiveBuilder::BuildPlane(const PrimitiveSpecification& spec, std::vector<float>& vertices, std::vector<uint32_t>& indices)
    {
        uint32_t wSegments = std::max(1u, spec.Segments[0]);
        uint32_t hSegments = std::max(1u, spec.Segments[2]);
        
        float wSize = spec.Size.x;
        float hSize = spec.Size.z;

        glm::vec3 normal(0.0f, 1.0f, 0.0f);
        glm::vec3 tangent(1.0f, 0.0f, 0.0f);
        glm::vec3 bitangent(0.0f, 0.0f, 1.0f);

        for (uint32_t y = 0; y <= hSegments; ++y) {
            for (uint32_t x = 0; x <= wSegments; ++x) {
                float u = (float)x / (float)wSegments;
                float v = (float)y / (float)hSegments;

                glm::vec3 pos(
                    (u - 0.5f) * wSize, 
                    0.0f, 
                    (v - 0.5f) * hSize
                );

                // Push attributes conditionally
                vertices.push_back(pos.x); vertices.push_back(pos.y); vertices.push_back(pos.z);
                if (spec.CalculateNormals) { vertices.push_back(normal.x); vertices.push_back(normal.y); vertices.push_back(normal.z); }
                vertices.push_back(u); vertices.push_back(v); // UVs
                if (spec.CalculateTangents) { vertices.push_back(tangent.x); vertices.push_back(tangent.y); vertices.push_back(tangent.z); }
                if (spec.CalculateBitangents) { vertices.push_back(bitangent.x); vertices.push_back(bitangent.y); vertices.push_back(bitangent.z); }
            }
        }

        for (uint32_t y = 0; y < hSegments; ++y) {
            for (uint32_t x = 0; x < wSegments; ++x) {
                uint32_t bottomLeft  = y * (wSegments + 1) + x;
                uint32_t bottomRight = bottomLeft + 1;
                uint32_t topLeft     = (y + 1) * (wSegments + 1) + x;
                uint32_t topRight    = topLeft + 1;

                indices.push_back(bottomLeft); indices.push_back(bottomRight); indices.push_back(topLeft);
                indices.push_back(bottomRight); indices.push_back(topRight); indices.push_back(topLeft);
            }
        }
    }

    void PrimitiveBuilder::BuildSphere(const PrimitiveSpecification& spec, std::vector<float>& vertices, std::vector<uint32_t>& indices)
    {
        uint32_t slices = std::max(3u, spec.Segments[0]); // Longitude
        uint32_t stacks = std::max(2u, spec.Segments[1]); // Latitude
        
        // Use Size.x as the radius
        float radius = spec.Size.x * 0.5f; 

        const float PI = glm::pi<float>();

        for (uint32_t y = 0; y <= stacks; ++y) {
            float v = (float)y / (float)stacks;
            float phi = v * PI; // 0 to PI

            for (uint32_t x = 0; x <= slices; ++x) {
                float u = (float)x / (float)slices;
                float theta = u * 2.0f * PI; // 0 to 2PI

                // 1. Position
                glm::vec3 pos(
                    std::cos(theta) * std::sin(phi) * radius,
                    std::cos(phi) * radius,
                    std::sin(theta) * std::sin(phi) * radius
                );

                // 2. Normal (normalized position from origin)
                glm::vec3 normal = glm::normalize(pos);

                // 3. Tangent (Derivative of position w.r.t theta)
                glm::vec3 tangent(-std::sin(theta), 0.0f, std::cos(theta));
                if (glm::length(tangent) > 0.0001f) tangent = glm::normalize(tangent);

                // 4. Bitangent
                glm::vec3 bitangent = glm::cross(normal, tangent);

                // Push
                vertices.push_back(pos.x); vertices.push_back(pos.y); vertices.push_back(pos.z);
                if (spec.CalculateNormals) { vertices.push_back(normal.x); vertices.push_back(normal.y); vertices.push_back(normal.z); }
                vertices.push_back(u); vertices.push_back(v);
                if (spec.CalculateTangents) { vertices.push_back(tangent.x); vertices.push_back(tangent.y); vertices.push_back(tangent.z); }
                if (spec.CalculateBitangents) { vertices.push_back(bitangent.x); vertices.push_back(bitangent.y); vertices.push_back(bitangent.z); }
            }
        }

        // Indices
        for (uint32_t y = 0; y < stacks; ++y) {
            for (uint32_t x = 0; x < slices; ++x) {
                uint32_t bottomLeft  = y * (slices + 1) + x;
                uint32_t bottomRight = bottomLeft + 1;
                uint32_t topLeft     = (y + 1) * (slices + 1) + x;
                uint32_t topRight    = topLeft + 1;

                indices.push_back(bottomLeft); indices.push_back(bottomRight); indices.push_back(topLeft);
                indices.push_back(bottomRight); indices.push_back(topRight); indices.push_back(topLeft);
            }
        }
    }

    void PrimitiveBuilder::BuildCube(const PrimitiveSpecification& spec, std::vector<float>& vertices, std::vector<uint32_t>& indices)
    {
        // Helper to build one face of the cube at a time, allowing for segments per face
        auto buildFace = [&](glm::vec3 center, glm::vec3 right, glm::vec3 up, uint32_t segX, uint32_t segY) {
            glm::vec3 normal = glm::normalize(glm::cross(right, up));
            glm::vec3 tangent = glm::normalize(right);
            glm::vec3 bitangent = glm::normalize(up); // or cross(normal, tangent)

            uint32_t startIndex = vertices.size() / (3 + (spec.CalculateNormals?3:0) + 2 + (spec.CalculateTangents?3:0) + (spec.CalculateBitangents?3:0));

            for (uint32_t y = 0; y <= segY; ++y) {
                for (uint32_t x = 0; x <= segX; ++x) {
                    float u = (float)x / (float)segX;
                    float v = (float)y / (float)segY;

                    glm::vec3 pos = center 
                                  + right * ((u - 0.5f) * spec.Size.x) 
                                  + up * ((v - 0.5f) * spec.Size.y);

                    vertices.push_back(pos.x); vertices.push_back(pos.y); vertices.push_back(pos.z);
                    if (spec.CalculateNormals) { vertices.push_back(normal.x); vertices.push_back(normal.y); vertices.push_back(normal.z); }
                    vertices.push_back(u); vertices.push_back(v);
                    if (spec.CalculateTangents) { vertices.push_back(tangent.x); vertices.push_back(tangent.y); vertices.push_back(tangent.z); }
                    if (spec.CalculateBitangents) { vertices.push_back(bitangent.x); vertices.push_back(bitangent.y); vertices.push_back(bitangent.z); }
                }
            }

            for (uint32_t y = 0; y < segY; ++y) {
                for (uint32_t x = 0; x < segX; ++x) {
                    uint32_t bottomLeft  = startIndex + y * (segX + 1) + x;
                    uint32_t bottomRight = bottomLeft + 1;
                    uint32_t topLeft     = startIndex + (y + 1) * (segX + 1) + x;
                    uint32_t topRight    = topLeft + 1;

                    indices.push_back(bottomLeft); indices.push_back(bottomRight); indices.push_back(topLeft);
                    indices.push_back(bottomRight); indices.push_back(topRight); indices.push_back(topLeft);
                }
            }
        };

        uint32_t sx = std::max(1u, spec.Segments[0]);
        uint32_t sy = std::max(1u, spec.Segments[1]);
        uint32_t sz = std::max(1u, spec.Segments[2]);
        float hx = spec.Size.x * 0.5f;
        float hy = spec.Size.y * 0.5f;
        float hz = spec.Size.z * 0.5f;

        // Build 6 faces
        buildFace(glm::vec3(0, 0, hz), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), sx, sy); // Front
        buildFace(glm::vec3(0, 0, -hz), glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0), sx, sy); // Back
        buildFace(glm::vec3(-hx, 0, 0), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0), sz, sy); // Left
        buildFace(glm::vec3(hx, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0), sz, sy); // Right
        buildFace(glm::vec3(0, hy, 0), glm::vec3(1, 0, 0), glm::vec3(0, 0, -1), sx, sz); // Top
        buildFace(glm::vec3(0, -hy, 0), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1), sx, sz); // Bottom
    }

    void PrimitiveBuilder::BuildCylinder(const PrimitiveSpecification& spec, std::vector<float>& vertices, std::vector<uint32_t>& indices)
    {
        uint32_t radialSegments = std::max(3u, spec.Segments[0]);
        uint32_t heightSegments = std::max(1u, spec.Segments[1]);
        
        float radius = spec.Size.x * 0.5f;
        float height = spec.Size.y;
        float halfHeight = height * 0.5f;
        const float PI = glm::pi<float>();

        // 1. Build the tubular side
        uint32_t startVertIndex = 0;
        for (uint32_t y = 0; y <= heightSegments; ++y) {
            float v = (float)y / (float)heightSegments;
            float yPos = -halfHeight + (v * height);

            for (uint32_t x = 0; x <= radialSegments; ++x) {
                float u = (float)x / (float)radialSegments;
                float theta = u * 2.0f * PI;

                glm::vec3 pos(std::cos(theta) * radius, yPos, std::sin(theta) * radius);
                glm::vec3 normal(std::cos(theta), 0.0f, std::sin(theta));
                glm::vec3 tangent(-std::sin(theta), 0.0f, std::cos(theta));
                glm::vec3 bitangent(0.0f, 1.0f, 0.0f); // Points straight up (+V)

                vertices.push_back(pos.x); vertices.push_back(pos.y); vertices.push_back(pos.z);
                if (spec.CalculateNormals) { vertices.push_back(normal.x); vertices.push_back(normal.y); vertices.push_back(normal.z); }
                vertices.push_back(u); vertices.push_back(v);
                if (spec.CalculateTangents) { vertices.push_back(tangent.x); vertices.push_back(tangent.y); vertices.push_back(tangent.z); }
                if (spec.CalculateBitangents) { vertices.push_back(bitangent.x); vertices.push_back(bitangent.y); vertices.push_back(bitangent.z); }
            }
        }

        for (uint32_t y = 0; y < heightSegments; ++y) {
            for (uint32_t x = 0; x < radialSegments; ++x) {
                uint32_t bottomLeft  = y * (radialSegments + 1) + x;
                uint32_t bottomRight = bottomLeft + 1;
                uint32_t topLeft     = (y + 1) * (radialSegments + 1) + x;
                uint32_t topRight    = topLeft + 1;

                indices.push_back(bottomLeft); indices.push_back(bottomRight); indices.push_back(topLeft);
                indices.push_back(bottomRight); indices.push_back(topRight); indices.push_back(topLeft);
            }
        }
    }

    void PrimitiveBuilder::BuildCapsule(const PrimitiveSpecification& spec, std::vector<float>& vertices, std::vector<uint32_t>& indices)
    {
        uint32_t radialSegments = std::max(3u, spec.Segments[0]); // Around the Y axis
        uint32_t heightSegments = std::max(1u, spec.Segments[1]); // Along the cylinder body
        uint32_t capSegments    = std::max(1u, spec.Segments[2]); // Along each hemisphere cap

        float radius = spec.Size.x * 0.5f;
        
        // H is the length of the internal cylinder. 
        // Total height is Size.y. If Size.y < diameter, H becomes 0 (it becomes a sphere).
        float H = std::max(0.0f, spec.Size.y - spec.Size.x); 

        const float PI = glm::pi<float>();
        const float HALF_PI = glm::half_pi<float>();

        // Helper struct to define the profile of the capsule before revolving it
        struct Ring {
            float y, r, ny, nr, v;
        };
        std::vector<Ring> rings;

        // Total surface arc length from pole to pole (used to scale the V texture coordinate evenly)
        float totalArc = PI * radius + H;
        
        // 1. Bottom Cap (from south pole up to equator)
        for (uint32_t i = 0; i <= capSegments; ++i) {
            float t = (float)i / capSegments;
            float phi = PI - t * HALF_PI; // PI up to PI/2
            
            Ring ring;
            ring.y = -H * 0.5f + radius * std::cos(phi);
            ring.r = radius * std::sin(phi);
            ring.ny = std::cos(phi);
            ring.nr = std::sin(phi);
            ring.v = (t * HALF_PI * radius) / totalArc;
            rings.push_back(ring);
        }
        
        // 2. Cylinder Body (from bottom of body up to top of body)
        for (uint32_t i = 1; i <= heightSegments; ++i) {
            float t = (float)i / heightSegments;
            
            Ring ring;
            ring.y = -H * 0.5f + t * H;
            ring.r = radius;
            ring.ny = 0.0f;
            ring.nr = 1.0f;
            ring.v = (HALF_PI * radius + t * H) / totalArc;
            rings.push_back(ring);
        }
        
        // 3. Top Cap (from equator up to north pole)
        for (uint32_t i = 1; i <= capSegments; ++i) {
            float t = (float)i / capSegments;
            float phi = HALF_PI - t * HALF_PI; // PI/2 down to 0
            
            Ring ring;
            ring.y = H * 0.5f + radius * std::cos(phi);
            ring.r = radius * std::sin(phi);
            ring.ny = std::cos(phi);
            ring.nr = std::sin(phi);
            ring.v = (HALF_PI * radius + H + t * HALF_PI * radius) / totalArc;
            rings.push_back(ring);
        }

        // Generate all vertices by revolving the rings around the Y axis
        for (const Ring& ring : rings) {
            for (uint32_t x = 0; x <= radialSegments; ++x) {
                float u = (float)x / radialSegments;
                float theta = u * 2.0f * PI;

                glm::vec3 pos(ring.r * std::cos(theta), ring.y, ring.r * std::sin(theta));
                glm::vec3 normal(ring.nr * std::cos(theta), ring.ny, ring.nr * std::sin(theta));
                
                glm::vec3 tangent(-std::sin(theta), 0.0f, std::cos(theta));
                if (glm::length(tangent) > 0.0001f) {
                    tangent = glm::normalize(tangent);
                } else {
                    tangent = glm::vec3(1.0f, 0.0f, 0.0f); // Fallback for the exact poles
                }

                glm::vec3 bitangent = glm::cross(normal, tangent);

                // Push Attributes
                vertices.push_back(pos.x); vertices.push_back(pos.y); vertices.push_back(pos.z);
                if (spec.CalculateNormals) {
                    vertices.push_back(normal.x); vertices.push_back(normal.y); vertices.push_back(normal.z);
                }
                vertices.push_back(u); vertices.push_back(ring.v);
                if (spec.CalculateTangents) {
                    vertices.push_back(tangent.x); vertices.push_back(tangent.y); vertices.push_back(tangent.z);
                }
                if (spec.CalculateBitangents) {
                    vertices.push_back(bitangent.x); vertices.push_back(bitangent.y); vertices.push_back(bitangent.z);
                }
            }
        }

        // Generate Triangulated Indices
        uint32_t ringCount = rings.size();
        for (uint32_t y = 0; y < ringCount - 1; ++y) {
            for (uint32_t x = 0; x < radialSegments; ++x) {
                uint32_t bottomLeft  = y * (radialSegments + 1) + x;
                uint32_t bottomRight = bottomLeft + 1;
                uint32_t topLeft     = (y + 1) * (radialSegments + 1) + x;
                uint32_t topRight    = topLeft + 1;

                indices.push_back(bottomLeft);
                indices.push_back(bottomRight);
                indices.push_back(topLeft);

                indices.push_back(bottomRight);
                indices.push_back(topRight);
                indices.push_back(topLeft);
            }
        }
    }

}