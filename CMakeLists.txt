cmake_minimum_required(VERSION 3.10)

project(cgproxy VERSION 3.4)
add_executable(cgattach cgattach.cpp)

install(TARGETS cgattach DESTINATION /usr/bin 
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE SETUID)
install(FILES  cgproxy.sh DESTINATION /usr/bin 
        RENAME cgproxy
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
install(FILES  cgnoproxy.sh DESTINATION /usr/bin 
        RENAME cgnoproxy
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
# install(FILES  run_in_cgroup.sh DESTINATION /usr/bin 
#         RENAME run_in_cgroup
#        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

install(FILES cgproxy.service 
        DESTINATION /usr/lib/systemd/system/)
install(FILES cgproxy.conf 
        DESTINATION /etc/)
install(FILES cgroup-tproxy.sh 
        DESTINATION /usr/share/cgproxy/scripts/)


## deb pack
set(CPACK_GENERATOR "DEB;RPM")
set(CPACK_PACKAGE_NAME "cgproxy")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "cgproxy will transparent proxy anything running in specific cgroup.It aslo supports global transparent proxy and gateway proxy")

set(CPACK_DEBIAN_PACKAGE_NAME "cgproxy")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "x86_64")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "systemd")
set(CPACK_DEBIAN_PACKAGE_SECTION "network")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "Optional")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/springzfx/cgproxy")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "springzfx@gmail.com")
set(CONTROL_DIR ${CMAKE_SOURCE_DIR}/control)
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CONTROL_DIR}/postinst;${CONTROL_DIR}/prerm")

set(CPACK_RPM_PACKAGE_ARCHITECTURE, "x86_64")
set(CPACK_RPM_PACKAGE_REQUIRES "systemd")
set(CPACK_RPM_PACKAGE_GROUP "network")
set(CPACK_RPM_PACKAGE_URL "https://github.com/springzfx/cgproxy")
set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${CONTROL_DIR}/postinst")
set(CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE "${CONTROL_DIR}/prerm")

include(CPack)