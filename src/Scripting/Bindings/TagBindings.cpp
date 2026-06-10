#include "TagBindings.h"

#include "pch.h"
#include "Scene/Components.h"

namespace Engine {

    void TagBindings::Register(sol::state& state)
    {
        state.new_usertype<TagComponent>("TagComponent",
            "Tag", &TagComponent::tag
        );
    }

}
