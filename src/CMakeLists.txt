find_package(Threads REQUIRED)
find_package(nlohmann_json REQUIRED)
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/execsnoop-libbpf/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(main main.cpp
                    common.cpp config.cpp cgroup_attach.cpp
                    socket_client.cpp socket_server.cpp)
target_link_libraries(main PRIVATE nlohmann_json::nlohmann_json Threads::Threads ${CMAKE_DL_LIBS})
set_target_properties(main PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(main PROPERTIES OUTPUT_NAME cgproxy)

install(TARGETS main DESTINATION /usr/bin PERMISSIONS ${basic_permission})
