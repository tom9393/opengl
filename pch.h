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
#include <math.h>
#include <list>
#include <random>
#include <mutex>
#include <functional>
#include <algorithm>
#include <memory>
#include <utility>
#include <unordered_map>
#include <filesystem>
#include <initializer_list>

#include "lib/glad/glad.h"
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/fwd.hpp>

#include "lib/imgui.h"
#include "lib/imgui_internal.h"
#include "lib/imgui_impl_glfw.h"
#include "lib/imgui_impl_opengl3.h"

#include "lib/ImGuizmo.h"

#include "lib/stb/stb_image.h"

#include "include/Instrumentor.h"
#include "include/Preprocessor.h"
