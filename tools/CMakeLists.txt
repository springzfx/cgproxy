include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src)
add_executable(cgattach cgattach.cpp)
install(TARGETS cgattach  DESTINATION /usr/bin PERMISSIONS ${basic_permission})