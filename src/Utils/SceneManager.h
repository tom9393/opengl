#pragma once

#include "../Scene/Entity.h"
#include "../Scene/Scene.h"

namespace Engine {

    enum class SceneState
    {
        Edit = 0,
        Play = 1
    };

    class SceneManager {

        public:
        static Entity GetSelectedEntity() {
            return { s_SelectedEntity, s_Context };

        }
        static entt::entity GetSelectedEntityHandle() { return s_SelectedEntity; }
        static Scene* GetContext() { return s_Context; }
        static SceneState GetSceneState() { return s_SceneState; }

        static void SetSelectedEntity(entt::entity entity) { s_SelectedEntity = entity; }
        static void SetContext(Scene* scene) { s_Context = scene; }
        static void SetSceneState(SceneState state) { s_SceneState = state; }
        static bool HasSelectedEntity() { return s_SelectedEntity != entt::null; }

        private:
        inline static entt::entity s_SelectedEntity = entt::null;
        inline static Scene* s_Context = nullptr;
        inline static SceneState s_SceneState = SceneState::Edit;
    };
}