include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_BINARY_DIR}/src)

find_package(nlohmann_json REQUIRED)
add_executable(client_test socket_client_test.cpp
        ../src/socket_client.cpp ../src/common.cpp ../src/config.cpp)
target_link_libraries(client_test nlohmann_json::nlohmann_json)