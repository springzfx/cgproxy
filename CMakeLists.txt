cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(cgproxy VERSION 0.13)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-unused-result")

set(build_tools OFF)
set(build_test  OFF)

set(basic_permission OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

add_subdirectory(src)
add_subdirectory(pack)
if (build_tools)
    add_subdirectory(tools)
endif()
if (build_test)
    add_subdirectory(test)
endif()

install(FILES cgproxyd DESTINATION /usr/bin PERMISSIONS ${basic_permission})
install(FILES cgnoproxy DESTINATION /usr/bin PERMISSIONS ${basic_permission})
install(FILES cgproxy.service DESTINATION /usr/lib/systemd/system/)
install(FILES config.json DESTINATION /etc/cgproxy/)
install(FILES cgroup-tproxy.sh DESTINATION /usr/share/cgproxy/scripts/ PERMISSIONS ${basic_permission})
install(FILES execsnoop.py DESTINATION /usr/share/cgproxy/scripts/ PERMISSIONS ${basic_permission})
install(FILES readme.md DESTINATION /usr/share/doc/cgproxy/)

# man pages
set(man_gz
${PROJECT_BINARY_DIR}/cgproxyd.1.gz
${PROJECT_BINARY_DIR}/cgproxy.1.gz
${PROJECT_BINARY_DIR}/cgnoproxy.1.gz
)
add_custom_target(man
    COMMAND gzip -fk cgproxyd.1 cgproxy.1 cgnoproxy.1
    COMMAND mv *.gz ${PROJECT_BINARY_DIR}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/man
)
add_dependencies(main man)
install(FILES ${man_gz} DESTINATION /usr/share/man/man1/)
