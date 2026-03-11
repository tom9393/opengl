#pragma once

#include "NativeScript.h"
namespace Engine {

    class TestScript : public NativeScript {
            void OnCreate() override {
            std::cout << "Hello from the script" << std::endl;
    }
    };
}
