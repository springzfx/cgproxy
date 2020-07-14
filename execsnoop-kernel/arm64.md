##  Cross Compile

```bash
docker pull debian:buster

# in container
dpkg --add-architecture arm64
apt update
apt install gcc-8-aarch64-linux-gnu # cross compile toolchain
apt install libelf-dev:arm64 # target depency library
...
```

## Emulation

### Register qemu-user-static

- before register binfmt

```bash
docker run --rm -t arm64/ubuntu uname -m
```

- register binfmt

```bash
# through docker
docker run --rm --privileged multiarch/qemu-user-static --reset -p yes
# or through systemd
pacman -S qemu-user-static binfmt-qemu-static
systemctl restart systemd-binfmt.service
# test
cat /proc/sys/fs/binfmt_misc/qemu-aarch64
```

- after register binfmt

```bash
docker run --rm -t arm64/ubuntu uname -m
```

### M1: Docker

```bash
# start container background
docker run -dit --name arm64 -v /home/fancy/workspace-xps:/data arm64v8/ubuntu

# enter container
docker exec -it arm64 bash
```

### M2: Chroot

download image [ubuntu-base-20.04-base-arm64.tar.gz](http://cdimage.ubuntu.com/ubuntu-base/releases/20.04/release/ubuntu-base-20.04-base-arm64.tar.gz), extract and chroot to it.

```bash
sudo arch-chroot ubuntu-base-20.04-base-arm64
```

### Refer

- https://www.stereolabs.com/docs/docker/building-arm-container-on-x86/

- https://github.com/junaruga/fedora-workshop-multiarch/blob/master/slides/Lets-add-Fedora-multiarch-to-CI.pdf

- https://wiki.debian.org/QemuUserEmulation

### Compile

ready some depencies.

```bash
# maybe repository: https://mirrors.tuna.tsinghua.edu.cn/help/ubuntu/
# install in container for kernel bpf build
apt install dialog apt-utils
apt install build-essential gcc clang llvm
apt install bison flex bc rsync libssl-dev binutils-dev libreadline-dev libelf-dev
apt install make cmake
# for cgproxy
apt install nlohmann-json3-dev rpm
```

