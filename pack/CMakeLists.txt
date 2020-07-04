## package for deb and rpm
set(CPACK_GENERATOR "DEB;RPM")
set(CPACK_PACKAGE_NAME "cgproxy")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "cgproxy will transparent proxy anything running in specific cgroup.It aslo supports global transparent proxy and gateway proxy")

## deb pack
set(CPACK_DEBIAN_PACKAGE_NAME "cgproxy")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "systemd, libbpf0")
set(CPACK_DEBIAN_PACKAGE_SECTION "network")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "Optional")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/springzfx/cgproxy")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "springzfx@gmail.com")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/postinst;${CMAKE_CURRENT_SOURCE_DIR}/prerm")

## rpm pack
set(CPACK_RPM_PACKAGE_ARCHITECTURE, "x86_64")
set(CPACK_RPM_PACKAGE_REQUIRES "systemd, libbpf")
set(CPACK_RPM_PACKAGE_GROUP "network")
set(CPACK_RPM_PACKAGE_URL "https://github.com/springzfx/cgproxy")
set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/postinst")
set(CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/prerm")

include(CPack)
