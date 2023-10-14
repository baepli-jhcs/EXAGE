// dear imgui: Platform Backend for GLFW
// This needs to be used along with a Renderer (e.g. OpenGL3, Vulkan, WebGPU..)
// (Info: GLFW is a cross-platform general purpose library for handling windows, inputs,
// OpenGL/Vulkan graphics context creation, etc.) (Requires: GLFW 3.1+. Prefer GLFW 3.3+ for full
// feature support.)

// Implemented features:
//  [X] Platform: Clipboard support.
//  [X] Platform: Keyboard support. Since 1.87 we are using the io.AddKeyEvent() function. Pass
//  ImGuiKey values to all key functions e.g. ImGui::IsKeyPressed(ImGuiKey_Space). [Legacy
//  GLFW_KEY_* values will also be supported unless IMGUI_DISABLE_OBSOLETE_KEYIO is set] [X]
//  Platform: Gamepad support. Enable with 'io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad'.
//  [x] Platform: Mouse cursor shape and visibility. Disable with 'io.ConfigFlags |=
//  ImGuiConfigFlags_NoMouseCursorChange' (note: the resizing cursors requires GLFW 3.4+). [X]
//  Platform: Multi-viewport support (multiple windows). Enable with 'io.ConfigFlags |=
//  ImGuiConfigFlags_ViewportsEnable'.

// Issues:
//  [ ] Platform: Multi-viewport support: ParentViewportID not honored, and so
//  io.ConfigViewportsNoDefaultParent has no effect (minor).

// You can use unmodified imgui_impl_* files in your project. See examples/ folder for examples of
// using this. Prefer including the entire imgui/ repository into your project (either as a copy or
// as a submodule), and only build the backends you need. If you are new to Dear ImGui, read
// documentation from the docs/ folder + read the top of imgui.cpp. Read online:
// https://github.com/ocornut/imgui/tree/master/docs

#pragma once
// NOLINTBEGIN

#include "exage/System/Event.h"
#include "exage/platform/GLFW/GLFWindow.h"
#include "imgui.h"  // IMGUI_IMPL_API

struct GLFWwindow;
struct GLFWmonitor;

IMGUI_IMPL_API bool ImGui_ImplGlfw_InitForOpenGL(exage::System::GLFWindow* window);
IMGUI_IMPL_API bool ImGui_ImplGlfw_InitForVulkan(exage::System::GLFWindow* window);
IMGUI_IMPL_API bool ImGui_ImplGlfw_InitForOther(exage::System::GLFWindow* window);
IMGUI_IMPL_API void ImGui_ImplGlfw_Shutdown();
IMGUI_IMPL_API void ImGui_ImplGlfw_NewFrame();
IMGUI_IMPL_API void ImGui_ImplGlfw_ProcessEvent(const exage::System::Event& event);

// NOLINTEND
