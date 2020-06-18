include_directories(${PROJECT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(execsnoop MODULE execsnoop.cpp ../src/common.cpp)
target_link_libraries(execsnoop bcc)
install(TARGETS execsnoop DESTINATION /usr/lib/cgproxy/ PERMISSIONS ${basic_permission})
