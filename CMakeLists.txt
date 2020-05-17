cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(cgproxy VERSION 4.0)
set(build_tools OFF)
# set(build_tools ON)
set(build_test  OFF)
# set(build_test  ON)

set(basic_permission OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

add_subdirectory(src)
if (build_tools)
    add_subdirectory(tools)
endif()
if (build_test)
    add_subdirectory(test)
endif()


install(FILES cgnoproxy DESTINATION /usr/bin PERMISSIONS ${basic_permission})
install(FILES cgproxy.service DESTINATION /usr/lib/systemd/system/)
install(FILES config.json DESTINATION /etc/cgproxy/)
install(FILES cgroup-tproxy.sh DESTINATION /usr/share/cgproxy/scripts/)
install(FILES readme.md DESTINATION /share/doc/cgproxy/)

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
