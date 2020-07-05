# find libbpf
if (build_static)
    find_library(LIBBPF libbpf.a)
else()
    find_library(LIBBPF bpf)
endif()

if (LIBBPF-NOTFOUND)
    message(FATAL_ERROR "libbpf not found")
endif()

if (build_execsnoop_dl)
    add_library(execsnoop MODULE execsnoop_share.cpp)
    install(TARGETS execsnoop DESTINATION ${CMAKE_INSTALL_LIBDIR}/cgproxy/)
else()
    add_library(execsnoop execsnoop_share.cpp)
endif()

target_link_libraries(execsnoop PRIVATE ${LIBBPF} -lelf -lz)