#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM

#include <iostream>
#include <stdio.h>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <array>
#include <math.h>
#include <cmath>
#include <list>
#include <random>
#include <future>
#include <mutex>
#include <functional>
#include <algorithm>
#include <memory>
#include <utility>
#include <unordered_map>
#include <filesystem>
#include <initializer_list>
#include <variant>
#include <thread>
#include <typeindex>
#include <atomic>
#include <queue>
#include <chrono>
#include <condition_variable>
#include <stdexcept>
#include <cstdio>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include <imguizmo/ImGuizmo.h>

#include <stb/stb_image.h>

#include "lib/entt/single_include/entt/entt.hpp"

#include "src/Utils/Instrumentor.h"
#include "src/Core/Preprocessor.h"
