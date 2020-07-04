# find libbpf
find_library(LIBBPF bpf)
if (LIBBPF-NOTFOUND)
    message(FATAL_ERROR "libbpf not found")
endif()

add_library(execsnoop MODULE execsnoop_share.cpp)
target_link_libraries(execsnoop PRIVATE bpf)
install(TARGETS execsnoop DESTINATION ${CMAKE_INSTALL_LIBDIR}/cgproxy/)