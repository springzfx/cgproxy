include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src)

add_executable(cgattach cgattach.cpp ../src/cgroup_attach.cpp ../src/common.cpp)
install(TARGETS cgattach  DESTINATION /usr/bin PERMISSIONS ${basic_permission})

if (with_execsnoop)
add_executable(execsnoop_exec execsnoop.cpp ../src/common.cpp ../src/execsnoop.cpp)
set_target_properties(execsnoop_exec PROPERTIES OUTPUT_NAME execsnoop)
target_link_libraries(execsnoop_exec bcc)
install(TARGETS execsnoop_exec  DESTINATION /usr/bin PERMISSIONS ${basic_permission})
endif()