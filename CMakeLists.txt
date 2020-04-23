cmake_minimum_required(VERSION 3.10)

project(cgproxy VERSION 1.0)
add_executable(cgattach cgattach.cpp)

install(TARGETS cgattach DESTINATION /usr/bin 
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE SETUID)
install(FILES  cgproxy.sh DESTINATION /usr/bin 
        RENAME cgproxy
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
install(FILES  nocgproxy.sh DESTINATION /usr/bin 
        RENAME nocgproxy
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
