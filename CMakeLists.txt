# CMakeList.txt : CMake project for Jubes, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.13)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Jubes")

include(FetchContent)
FetchContent_Declare(
	glfw
	GIT_REPOSITORY "https://github.com/glfw/glfw"
	GIT_TAG "7b6aead9fb88b3623e3b3725ebb42670cbe4c579"
)
FetchContent_Declare(
	fmt
	GIT_REPOSITORY "https://github.com/fmtlib/fmt"
	GIT_TAG "0c9fce2ffefecfdce794e1859584e25877b7b592"
)

FetchContent_MakeAvailable(glfw fmt)
find_package(Vulkan)

# Add source to this project's executable.
add_executable (Jubes)
add_subdirectory("src")

target_link_libraries(Jubes PRIVATE glfw fmt Vulkan::Vulkan)
if (MSVC)
	target_compile_options(Jubes PRIVATE /W4)
else()
	target_compile_options(Jubes PRIVATE -Wall -Wextra -Wpedantic)
endif()

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Jubes PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.

