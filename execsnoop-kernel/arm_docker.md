## Arm64 docker

https://www.stereolabs.com/docs/docker/building-arm-container-on-x86/

https://wiki.debian.org/QemuUserEmulation

- install

```bash
# install qemu with user emulation
pacman -S qemu qemu-user-static
# docker
docker pull arm64v8/ubuntu
docker pull multiarch/qemu-user-static
# register
docker run --rm --privileged multiarch/qemu-user-static --reset -p yes
```

- test

```bash
docker run --rm -t arm64/ubuntu uname -m
```

- run

```bash
# start container background
docker run -dit --name arm64 -v /home/fancy/workspace-xps:/data arm64v8/ubuntu

# enter container
docker exec -it arm64 bash
# use another repository: https://mirrors.tuna.tsinghua.edu.cn/help/ubuntu/
# install in container for kernel bpf build
apt install install dialog apt-utils
apt install build-essential gcc clang llvm
apt install bison flex bc rsync libssl-dev binutils-dev libreadline-dev libelf 
apt install make cmake nlohmann-json3-dev rpm
```

