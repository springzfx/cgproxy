add_library(execsnoop MODULE execsnoop_share.cpp)
target_link_libraries(execsnoop PRIVATE bpf)
install(TARGETS execsnoop DESTINATION ${CMAKE_INSTALL_LIBDIR}/cgproxy/)