include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src)

add_executable(cgattach cgattach.cpp ../src/cgroup_attach.cpp ../src/common.cpp)
install(TARGETS cgattach  DESTINATION /usr/bin PERMISSIONS ${basic_permission})

add_executable(execsnoop execsnoop.cpp ../src/common.cpp)
target_link_libraries(execsnoop bcc)
install(TARGETS execsnoop  DESTINATION /usr/bin PERMISSIONS ${basic_permission})