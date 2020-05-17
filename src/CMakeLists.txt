find_package(Threads REQUIRED)
find_package(nlohmann_json REQUIRED)
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(cgproxyd cgproxyd.cpp)
add_executable(cgproxy cgproxy.cpp)
target_link_libraries(cgproxyd PRIVATE Threads::Threads nlohmann_json::nlohmann_json)
target_link_libraries(cgproxy PRIVATE nlohmann_json::nlohmann_json)

install(TARGETS cgproxyd  DESTINATION /usr/bin PERMISSIONS ${basic_permission})
install(TARGETS cgproxy   DESTINATION /usr/bin PERMISSIONS ${basic_permission})
