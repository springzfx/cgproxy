find_package(nlohmann_json REQUIRED)
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/execsnoop-libbpf/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

configure_file(common.cmake.h ${CMAKE_CURRENT_SOURCE_DIR}/common.h)

if (build_execsnoop_dl)
    add_definitions(-DBUIlD_EXECSNOOP_DL)
    set(DL_LIB "-ldl")
    set(EXECSNOOP_LIB "")
else()
    set(EXECSNOOP_LIB "execsnoop")
endif()

add_executable(main main.cpp common.cpp config.cpp cgroup_attach.cpp socket_client.cpp socket_server.cpp)
target_link_libraries(main PRIVATE nlohmann_json::nlohmann_json ${DL_LIB} ${EXECSNOOP_LIB})
set_target_properties(main PROPERTIES OUTPUT_NAME cgproxy)
install(TARGETS main DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})

if (build_static)
    target_link_libraries(main PRIVATE -static -Wl,--whole-archive -lpthread -Wl,--no-whole-archive)
else()
    target_link_libraries(main PRIVATE -lpthread)
endif()
