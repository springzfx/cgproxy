find_library(LIBBPF bpf REQUIRED)

# generate execsnoop.skel.h
if (EXISTS /sys/kernel/btf/vmlinux)
add_custom_command(OUTPUT execsnoop.skel.h
    COMMAND bpftool btf dump file /sys/kernel/btf/vmlinux format c > vmlinux.h
    COMMAND clang -O2 -g -target bpf -c execsnoop.bpf.c -o execsnoop.bpf.o
    COMMAND bpftool gen skeleton execsnoop.bpf.o > execsnoop.skel.h
    DEPENDS execsnoop.bpf.c
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
endif()

if (build_execsnoop_dl)
    add_library(execsnoop MODULE execsnoop_share.cpp execsnoop.skel.h)
    target_link_libraries(execsnoop PRIVATE ${LIBBPF})
    install(TARGETS execsnoop DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/cgproxy/)
else()
    add_library(execsnoop execsnoop_share.cpp execsnoop.skel.h)
    target_link_libraries(execsnoop PRIVATE ${LIBBPF})
endif()
