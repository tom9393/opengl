#pragma once

#include "BulletCollision/CollisionShapes/btConvexHullShape.h"
#include "BulletCollision/CollisionShapes/btShapeHull.h"

#include "AssetManager.h"
#include "Mesh.h"
#include "Scene/Entity.h"

namespace Engine {

    class CollisionBaker
    {
    public:
    // Generates a simplified hull and saves it to disk
        static std::string Bake(Entity &entity)
        {
            std::vector<Shared<Mesh>> meshes;
            std::vector<Entity> meshEntities;

            if (entity.HasComponent<MeshComponent>()) meshEntities.emplace_back(entity);

            entity.GetAllChildrenWithComponent<MeshComponent>(meshEntities);
            for (auto& e : meshEntities)
            {
                UUID handle = e.GetComponent<MeshComponent>().MeshHandle;
                meshes.emplace_back(AssetManager::GetAsset<Mesh>(handle));
            }

            AssetMetadata metadata = AssetManager::GetMetadata(meshEntities.front().GetComponent<MeshComponent>().MeshHandle);

            std::string outputPath = metadata.FilePath + ".pmcol";

            if (std::filesystem::exists(outputPath)) return outputPath;
            std::ofstream out(outputPath, std::ios::binary);

            uint32_t meshCount = (uint32_t)meshes.size();
            out.write((char*)&meshCount, sizeof(uint32_t));

            for (const auto& mesh : meshes) {
                // 1. Create a temporary hull from raw mesh vertices
                btConvexHullShape tempShape;
                uint32_t stride = mesh->GetVBOs()[0]->GetLayout().GetStride() / sizeof(float);
                const std::vector<float>& verts = mesh->GetVertices();

                for (size_t i = 0; i < verts.size(); i += stride) {
                    tempShape.addPoint(btVector3(verts[i], verts[i+1], verts[i+2]));
                }

                // 2. Simplify the hull using Bullet's utility (The "Bake" step)
                btShapeHull hullScanner(&tempShape);
                hullScanner.buildHull(tempShape.getMargin());

                // 3. Save only the simplified vertex indices
                uint32_t vCount = (uint32_t)hullScanner.numVertices();
                const btVector3* hullVerts = hullScanner.getVertexPointer();

                out.write((char*)&vCount, sizeof(uint32_t));
                for (uint32_t i = 0; i < vCount; i++) {
                    glm::vec3 v = { hullVerts[i].x(), hullVerts[i].y(), hullVerts[i].z() };
                    out.write((char*)&v, sizeof(glm::vec3));
                }
            }
            out.close();
            std::cout << "[Baker] Finished baking: " << outputPath << std::endl;
            return outputPath;
        }
    };
}
