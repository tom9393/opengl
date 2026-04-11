#include "PrimitiveBuilder.h"
#include <vector>
#include <cmath>

namespace Engine {

    // Helper to streamline creating and registering the mesh
    static UUID RegisterMesh(const std::string& name, const std::vector<float>& vertices, const std::vector<uint32_t>& indices)
    {
        // 1. Create the Vertex Array
        Shared<VertexArray> vao = VertexArray::Create();

        // 2. Create the Vertex Buffers
        Shared<VertexBuffer> vbo = VertexBuffer::Create((float*)vertices.data(), vertices.size() * sizeof(float));
        Shared<VertexBuffer> vbo2 = VertexBuffer::Create(1000 * sizeof(glm::mat4));
        
        // Setup layout: Assumes 3 floats for Position, 3 for Normal, 2 for TexCoords.
        // (Adjust the enum/type names if your BufferLayout.h uses different naming)
        // BufferLayout layout = {
        //     { ShaderDataType::Float3, "a_Position" },
        //     { ShaderDataType::Float3, "a_Normal" },
        //     { ShaderDataType::Float2, "a_TexCoord" }
        // };
        BufferLayout layout = {
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float3, "a_Normal" },
            { ShaderDataType::Float2, "a_TexCoord" },
            { ShaderDataType::Float3, "a_Tangent" },
            { ShaderDataType::Float3, "a_Bitangent" }
        };
        BufferLayout instanceLayout = {
            { ShaderDataType::Mat4, "a_InstanceTransform" },
        };
        vbo->SetLayout(layout);
        vao->AddVertexBuffer(vbo);

        vbo2->SetLayout(instanceLayout);
        vao->AddVertexBuffer(vbo2);

        // 3. Create the Index Buffer
        Shared<IndexBuffer> ebo = IndexBuffer::Create((unsigned int*)indices.data(), indices.size());
        vao->SetIndexBuffer(ebo);

        std::vector<Shared<VertexBuffer>> vbos = { vbo, vbo2 };

        // 4. Construct your Mesh class
        Shared<Mesh> mesh = MakeShared<Mesh>(vertices, indices, vao, vbos, ebo); 

        glm::vec3 minAABB(std::numeric_limits<float>::max());
        glm::vec3 maxAABB(std::numeric_limits<float>::lowest());

        // Step through the flat array 8 floats at a time (since each vertex is 8 floats)
        for (size_t i = 0; i < vertices.size(); i += 8)
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
        // NOTE: Adjust .Min and .Max if your MathTools.h struct uses lowercase .min and .max
        aabb.Min = minAABB; 
        aabb.Max = maxAABB;
        mesh->SetAABB(aabb);

        // Register it as a memory asset (using the function we added previously)
        return AssetManager::CreateMemoryAsset(AssetType::Mesh, name, mesh);
    }

    UUID PrimitiveBuilder::CreatePlane()
    {
        std::vector<float> vertices = {
            // Position              // Normal              // TexCoords
            -0.5f, 0.0f, -0.5f,      0.0f, 1.0f, 0.0f,      0.0f, 1.0f,
             0.5f, 0.0f, -0.5f,      0.0f, 1.0f, 0.0f,      1.0f, 1.0f,
             0.5f, 0.0f,  0.5f,      0.0f, 1.0f, 0.0f,      1.0f, 0.0f,
            -0.5f, 0.0f,  0.5f,      0.0f, 1.0f, 0.0f,      0.0f, 0.0f
        };

        std::vector<unsigned int> indices = {
            0, 2, 1, 
            2, 0, 3  
        };

        return RegisterMesh("Plane", vertices, indices);
    }

    UUID PrimitiveBuilder::CreateCube()
    {
        // Standard 24-vertex cube (flattened array)
        float v[] = {
            // Back face
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, 
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, 
             0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, 
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, 
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, 
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, 
            // Front face
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, 
             0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, 
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, 
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, 
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, 
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, 
            // Left face
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, 
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, 
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, 
            // Right face
             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, 
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 
             0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, 
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 
             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, 
             0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 
            // Bottom face
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, 
             0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, 
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, 
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, 
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, 
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, 
            // Top face
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, 
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, 
             0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, 
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, 
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, 
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f
        };

        // Convert raw array to vector
        std::vector<float> vertices(v, v + sizeof(v) / sizeof(float));
        
        std::vector<unsigned int> indices(36);
        for (unsigned int i = 0; i < 36; i++) {
            indices[i] = i; // Already triangulated
        }

        return RegisterMesh("Cube", vertices, indices);
    }

    UUID PrimitiveBuilder::CreateSphere(uint32_t xSegments, uint32_t ySegments)
    {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        const float PI = 3.14159265359f;

        for (unsigned int y = 0; y <= ySegments; ++y) {
            for (unsigned int x = 0; x <= xSegments; ++x) {
                float xSegment = (float)x / (float)xSegments;
                float ySegment = (float)y / (float)ySegments;
                
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                // Push Position
                vertices.push_back(xPos);
                vertices.push_back(yPos);
                vertices.push_back(zPos);

                // Push Normal
                vertices.push_back(xPos);
                vertices.push_back(yPos);
                vertices.push_back(zPos);

                // Push TexCoords
                vertices.push_back(xSegment);
                vertices.push_back(ySegment);
            }
        }

        bool oddRow = false;
        for (unsigned int y = 0; y < ySegments; ++y) {
            for (unsigned int x = 0; x < xSegments; ++x) {
                indices.push_back((y + 1) * (xSegments + 1) + x);
                indices.push_back(y * (xSegments + 1) + x);
                indices.push_back(y * (xSegments + 1) + x + 1);

                indices.push_back((y + 1) * (xSegments + 1) + x);
                indices.push_back(y * (xSegments + 1) + x + 1);
                indices.push_back((y + 1) * (xSegments + 1) + x + 1);
            }
        }

        return RegisterMesh("Sphere", vertices, indices);
    }

}