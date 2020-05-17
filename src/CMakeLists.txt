find_package(Threads REQUIRED)
find_package(nlohmann_json REQUIRED)
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(cgproxy-lib
            SHARED  
            common.cpp
            config.cpp
            cgroup_attach.cpp
            socket_server.cpp socket_client.cpp
            )
set_target_properties(cgproxy-lib PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(cgproxy-lib PROPERTIES OUTPUT_NAME cgproxy)
target_link_libraries(cgproxy-lib nlohmann_json::nlohmann_json)

add_executable(cgproxyd cgproxyd.cpp)
add_executable(cgproxy cgproxy.cpp)
target_link_libraries(cgproxyd cgproxy-lib Threads::Threads nlohmann_json::nlohmann_json)
target_link_libraries(cgproxy  cgproxy-lib nlohmann_json::nlohmann_json)

install(TARGETS cgproxyd  DESTINATION /usr/bin PERMISSIONS ${basic_permission})
install(TARGETS cgproxy   DESTINATION /usr/bin PERMISSIONS ${basic_permission})
install(TARGETS cgproxy-lib   DESTINATION /usr/lib PERMISSIONS ${basic_permission})
