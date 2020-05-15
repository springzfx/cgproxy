cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_BUILD_TYPE RELEASE)

project(cgproxy VERSION 3.7)


find_package(Threads REQUIRED)
find_package(nlohmann_json REQUIRED)
include_directories(${PROJECT_SOURCE_DIR})

add_executable(cgattach cgattach.cpp)

add_executable(main main.cpp)
target_link_libraries(main Threads::Threads nlohmann_json::nlohmann_json)

add_executable(client_test socket_client_test.cpp)
target_link_libraries(client_test nlohmann_json::nlohmann_json)


install(TARGETS cgattach DESTINATION /usr/bin 
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE SETUID)
install(FILES  cgproxy.sh DESTINATION /usr/bin 
        RENAME cgproxy
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
install(FILES  cgnoproxy.sh DESTINATION /usr/bin 
        RENAME cgnoproxy
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

install(FILES cgproxy.service 
        DESTINATION /usr/lib/systemd/system/)
install(FILES cgproxy.json
        DESTINATION /etc/cgproxy/)
install(FILES cgroup-tproxy.sh 
        DESTINATION /usr/share/cgproxy/scripts/)
        
install(FILES readme.md
        DESTINATION /share/doc/cgproxy/)




## package for deb and rpm
set(CPACK_GENERATOR "DEB;RPM")
set(CPACK_PACKAGE_NAME "cgproxy")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "cgproxy will transparent proxy anything running in specific cgroup.It aslo supports global transparent proxy and gateway proxy")

## deb pack
set(CPACK_DEBIAN_PACKAGE_NAME "cgproxy")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "x86_64")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "systemd")
set(CPACK_DEBIAN_PACKAGE_SECTION "network")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "Optional")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/springzfx/cgproxy")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "springzfx@gmail.com")
set(CONTROL_DIR ${CMAKE_SOURCE_DIR}/control)
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CONTROL_DIR}/postinst;${CONTROL_DIR}/prerm")

## rpm pack
set(CPACK_RPM_PACKAGE_ARCHITECTURE, "x86_64")
set(CPACK_RPM_PACKAGE_REQUIRES "systemd")
set(CPACK_RPM_PACKAGE_GROUP "network")
set(CPACK_RPM_PACKAGE_URL "https://github.com/springzfx/cgproxy")
set(CONTROL_DIR ${CMAKE_SOURCE_DIR}/control)
set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${CONTROL_DIR}/postinst")
set(CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE "${CONTROL_DIR}/prerm")

include(CPack)
