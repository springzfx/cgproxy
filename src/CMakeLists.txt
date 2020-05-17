find_package(Threads REQUIRED)
find_package(nlohmann_json REQUIRED)
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(cgproxy-lib  SHARED config.cpp socket_client.cpp common.cpp)
set_target_properties(cgproxy-lib PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(cgproxy-lib PROPERTIES OUTPUT_NAME cgproxy)
target_link_libraries(cgproxy-lib nlohmann_json::nlohmann_json)

add_library(cgproxyd-lib SHARED config.cpp cgroup_attach.cpp socket_server.cpp common.cpp)
set_target_properties(cgproxyd-lib PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(cgproxyd-lib PROPERTIES OUTPUT_NAME cgproxyd)
target_link_libraries(cgproxyd-lib nlohmann_json::nlohmann_json)

add_executable(cgproxyd cgproxyd.cpp)
add_executable(cgproxy cgproxy.cpp) 
target_link_libraries(cgproxyd cgproxyd-lib Threads::Threads)
target_link_libraries(cgproxy  cgproxy-lib)

install(TARGETS cgproxyd  DESTINATION /usr/bin PERMISSIONS ${basic_permission})
install(TARGETS cgproxy   DESTINATION /usr/bin PERMISSIONS ${basic_permission})
install(TARGETS cgproxy-lib   DESTINATION /usr/lib PERMISSIONS ${basic_permission})
install(TARGETS cgproxyd-lib   DESTINATION /usr/lib PERMISSIONS ${basic_permission})
