## package for deb and rpm
set(CPACK_GENERATOR "DEB;RPM")
set(CPACK_PACKAGE_NAME "cgproxy")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "cgproxy will transparent proxy anything running in specific cgroup.It aslo supports global transparent proxy and gateway proxy")

## deb pack
execute_process(COMMAND dpkg --print-architecture 
    OUTPUT_VARIABLE DEBIAN_ARCH 
    OUTPUT_STRIP_TRAILING_WHITESPACE)
set(CPACK_DEBIAN_FILE_NAME ${CPACK_PACKAGE_NAME}_${CMAKE_PROJECT_VERSION}_${DEBIAN_ARCH}.deb)
set(CPACK_DEBIAN_PACKAGE_NAME "cgproxy")
# set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "systemd")
set(CPACK_DEBIAN_PACKAGE_SECTION "network")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "Optional")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/springzfx/cgproxy")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "springzfx@gmail.com")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/postinst;${CMAKE_CURRENT_SOURCE_DIR}/prerm")

## rpm pack
execute_process(COMMAND uname -m 
        OUTPUT_VARIABLE RPM_ARCH 
        OUTPUT_STRIP_TRAILING_WHITESPACE)
set(CPACK_RPM_FILE_NAME ${CPACK_PACKAGE_NAME}_${CMAKE_PROJECT_VERSION}_${RPM_ARCH}.rpm)
# set(CPACK_RPM_PACKAGE_ARCHITECTURE, "x86_64")
set(CPACK_RPM_PACKAGE_REQUIRES "systemd")
set(CPACK_RPM_PACKAGE_GROUP "network")
set(CPACK_RPM_PACKAGE_URL "https://github.com/springzfx/cgproxy")
set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/postinst")
set(CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/prerm")

include(CPack)
