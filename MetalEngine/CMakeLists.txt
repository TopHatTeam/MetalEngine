# CMakeList.txt : CMake project for MetalEngine, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (MetalEngine 
"MetalEngine.cpp" 
"MetalEngine.h" 
"src/MMath.cpp"
"src/MVulkanRenderer.cpp"
"src/MError.c")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET MetalEngine PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
target_link_libraries(${PROJECT_NAME} PRIVATE 
	Vulkan::Vulkan
	SDL3::SDL3
	fmt::fmt
)
