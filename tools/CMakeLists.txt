include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_BINARY_DIR}/src)

add_executable(cgattach cgattach.cpp ../src/cgroup_attach.cpp ../src/common.cpp)
install(TARGETS cgattach  DESTINATION ${CMAKE_INSTALL_FULL_BINDIR} PERMISSIONS ${basic_permission})