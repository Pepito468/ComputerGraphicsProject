include_directories(../source/src/engine)
include_directories(../source/src/engine/utils)
include_directories(../source/src/engine/cameras)
include_directories(../source/src/engine/colliders)
include_directories(../source/src/engine/lights)
include_directories(../source/src/engine/ui)
include_directories(../source/src/engine/rendering)
include_directories(../source/src/engine/audio/)
include_directories(../source/include/modules)
include_directories(../source/include)

project(Test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-fpermissive)

find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)

# To add a new test:
#   - Write the test file newTest.cpp in source/tests/
#       - If headers other than those in source/src/engine are used, add an include_directories(path/to/headers)
#   - Add in this file add_executable(newTest newTest.cpp)
#   - Add in this file add_test(uniqueTestName nodeTest)
#       - uniqueTestName is the name that will be shown in the terminal when running test.sh, nodeTest is the name of the executable found in build
#   - To see the output of the test, go to build/tests/Testing/Temporary/LastTest.log

add_executable(NodeTest NodeTest.cpp)
target_link_libraries(NodeTest PRIVATE glfw glm::glm)
target_link_libraries(NodeTest PRIVATE Vulkan::Vulkan)
add_test(NodeTest NodeTest)

add_executable(Node3DTest Node3DTest.cpp)
target_link_libraries(Node3DTest PRIVATE glfw glm::glm)
target_link_libraries(Node3DTest PRIVATE Vulkan::Vulkan)
add_test(Node3DTest Node3DTest)


add_executable(Node2DTest Node2DTest.cpp)
target_link_libraries(Node2DTest PRIVATE glfw glm::glm)
target_link_libraries(Node2DTest PRIVATE Vulkan::Vulkan)
add_test(Node2DTest Node2DTest)

add_executable(ColliderTest ColliderTest.cpp)
target_link_libraries(ColliderTest PRIVATE glfw glm::glm)
target_link_libraries(ColliderTest PRIVATE Vulkan::Vulkan)
add_test(ColliderTest ColliderTest)

add_executable(JSONTest JSONTest.cpp)
target_link_libraries(JSONTest PRIVATE glfw glm::glm)
target_link_libraries(JSONTest PRIVATE Vulkan::Vulkan)
add_test(JSONTest JSONTest)

add_executable(PhysicsTest PhysicsTest.cpp)
target_link_libraries(PhysicsTest PRIVATE glfw glm::glm)
target_link_libraries(PhysicsTest PRIVATE Vulkan::Vulkan)
add_test(PhysicsTest PhysicsTest)
