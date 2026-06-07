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
        // Bake a collision shape for given entity using its mesh tree's AABBs
        static void Bake(Entity &entity)
        {
            std::vector<Shared<Mesh>> meshes = GatherMeshes(entity);
            if (meshes.empty()) return;

            std::string path = PathForHash(HashMeshes(meshes));
            UUID uuid = entity.GetUUID();

            auto it = s_EntityShapes.find(uuid);
            std::string oldPath = (it != s_EntityShapes.end()) ? it->second : std::string();

            if (!std::filesystem::exists(path))
                WriteBake(path, meshes);

            s_EntityShapes[uuid] = path;

            if (!oldPath.empty() && oldPath != path)
                DeleteIfOrphaned(entity, oldPath);
        }

        static std::string GetShapePath(Entity &entity)
        {
            std::string path = ComputePath(entity);
            if (path.empty() || !std::filesystem::exists(path)) return "";

            s_EntityShapes[entity.GetUUID()] = path;
            return path;
        }

        static bool HasShape(Entity &entity)
        {
            return !GetShapePath(entity).empty();
        }

        // Call before destroying entity/removing component
        static void Release(Entity &entity)
        {
            auto it = s_EntityShapes.find(entity.GetUUID());
            if (it == s_EntityShapes.end()) return;

            std::string path = it->second;
            s_EntityShapes.erase(it);
            DeleteIfOrphaned(entity, path);
        }

    private:
        static inline std::unordered_map<UUID, std::string> s_EntityShapes;
        static constexpr const char* s_CollisionDir = "../assets/collision/";

        static std::vector<Shared<Mesh>> GatherMeshes(Entity &entity)
        {
            std::vector<Entity> meshEntities;
            if (entity.HasComponent<MeshComponent>()) meshEntities.emplace_back(entity);
            entity.GetAllChildrenWithComponent<MeshComponent>(meshEntities);

            std::vector<Shared<Mesh>> meshes;
            for (auto& e : meshEntities)
                meshes.emplace_back(AssetManager::GetAsset<Mesh>(e.GetComponent<MeshComponent>().MeshHandle));
            return meshes;
        }

        static std::string ComputePath(Entity &entity)
        {
            std::vector<Shared<Mesh>> meshes = GatherMeshes(entity);
            if (meshes.empty()) return "";
            return PathForHash(HashMeshes(meshes));
        }

        // FNV-1a
        static uint64_t HashMeshes(const std::vector<Shared<Mesh>> &meshes)
        {
            uint64_t hash = 14695981039346656037ULL;
            auto fold = [&](const void* data, size_t size) {
                const uint8_t* bytes = static_cast<const uint8_t*>(data);
                for (size_t i = 0; i < size; i++) { hash ^= bytes[i]; hash *= 1099511628211ULL; }
            };

            for (const auto& mesh : meshes) {
                const AABB& aabb = mesh->GetAABB();
                fold(&aabb.Min, sizeof(glm::vec3));
                fold(&aabb.Max, sizeof(glm::vec3));
            }
            return hash;
        }

        static std::string PathForHash(uint64_t hash)
        {
            return std::string(s_CollisionDir) + std::to_string(hash) + ".pmcol";
        }

        static void WriteBake(const std::string &path, const std::vector<Shared<Mesh>> &meshes)
        {
            std::filesystem::create_directories(s_CollisionDir);
            std::ofstream out(path, std::ios::binary);

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
            std::cout << "[Baker] Finished baking: " << path << std::endl;
        }

        // Drop a collision shape if no longer used by any entity
        static void DeleteIfOrphaned(Entity &entity, const std::string &path)
        {
            Scene* scene = entity.GetScene();
            if (!scene) return;

            auto view = scene->GetRegistry().view<RigidBodyComponent>();
            for (auto e : view) {
                if (e == entity.GetHandle()) continue;
                Entity other{ e, scene };
                if (ComputePath(other) == path) return;
            }

            std::filesystem::remove(path);
            std::cout << "[Baker] Removed orphaned collision: " << path << std::endl;
        }
    };
}
