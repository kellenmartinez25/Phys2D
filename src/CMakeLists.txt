set(DEPS glfw glm::glm glad::glad OpenGL::GL)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_executable (Phys2D "Phys2D.cpp" "shader.cpp" "resource_manager.cpp" "Engine.cpp" "Renderer.cpp" "PhysObject.cpp" "Collision.h" "Collision.cpp" "Dynamics.cpp" "Dynamics.h" "MathUtils.cpp" "MathUtils.h" "DebugTools.cpp" "PhysTypes.cpp")

target_link_libraries(Phys2D PRIVATE ${DEPS} -static)
target_include_directories(Phys2D PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_target(aaa ALL
	COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
	${CMAKE_SOURCE_DIR}/resources 
	${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources
	DEPENDS ${MY_TARGET})
