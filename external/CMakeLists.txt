# Suppress all warnings from third-party projects.
add_definitions(-w)

add_subdirectory(VulkanMemoryAllocator-Hpp)
add_subdirectory(imgui)

add_subdirectory(slot_map)
add_subdirectory(tinyfiledialogs)