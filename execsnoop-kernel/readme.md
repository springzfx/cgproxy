
## Prons and cons

- use stable execve tracepoint, so build once and should work everywhere
- need to build in kernel tree

## Build in kernel tree

- download kernel source code
- ready and config kernel tree

```bash
# kernel config
#gunzip -c /proc/config.gz > .config
#make oldconfig && make prepare
make defconfig && make prepare
# install headers to ./usr/include
make headers_install -j8
# build samples/bpf
make samples/bpf -j8
# build bpftool
make tools/bpf -j8
```

- put or link `execsnoop_kern.c` and `execsnoop_user.c` to *samples/bpf/*
- edit  *samples/bpf/makefile*

```makefile
# in samples/bpf/makefile
tprogs-y += execsnoop
execsnoop-objs := bpf_load.o execsnoop_user.o $(TRACE_HELPERS)
always-y += execsnoop_kern.o
```

- compile again

```
make M=samples/bpf -j8
```

- run test

```bash
cd samples/bpf
sudo bash -c "ulimit -l unlimited && ./execsnoop"
```

## With bpftool

- generate `execsnoop_kern_skel.h`

```
bpftool gen skeleton execsnoop_kern.o > execsnoop_kern_skel.h
```

- build execsnoop

```
gcc -Wall -O2 execsnoop_user_1.c -o execsnoop -lbpf
```

## Detail build command

using `make V=1 M=samples/bpf | tee -a log.txt` to get and filter following command

- build `execsnoop_kern.o`

  note `-g` is needed if with BPF CO-RE

```bash
clang -nostdinc \
	-isystem /usr/lib/gcc/x86_64-pc-linux-gnu/10.1.0/include \
	-I./arch/x86/include \
	-I./arch/x86/include/generated  \
	-I./include \
	-I./arch/x86/include/uapi \
	-I./arch/x86/include/generated/uapi \
	-I./include/uapi \
	-I./include/generated/uapi \
	-include ./include/linux/kconfig.h \
	-I./samples/bpf \
	-I./tools/testing/selftests/bpf/ \
	-I./tools/lib/ \
	-include asm_goto_workaround.h \
	-D__KERNEL__ -D__BPF_TRACING__ -Wno-unused-value -Wno-pointer-sign \
	-D__TARGET_ARCH_x86 -Wno-compare-distinct-pointer-types \
	-Wno-gnu-variable-sized-type-not-at-end \
	-Wno-address-of-packed-member -Wno-tautological-compare \
	-Wno-unknown-warning-option  \
	-fno-stack-protector \
	-O2 -emit-llvm -c samples/bpf/execsnoop_kern.c \
	-o - | llc -march=bpf  -filetype=obj -o samples/bpf/execsnoop_kern.o
```

- build `execsnoop_user.o`

```bash
  gcc -Wall -O2 -Wmissing-prototypes -Wstrict-prototypes \
  	-I./usr/include \
  	-I./tools/testing/selftests/bpf/ \
  	-I./tools/lib/ \
  	-I./tools/include \
  	-I./tools/perf \
  	-DHAVE_ATTR_TEST=0  \
  	-c -o samples/bpf/execsnoop_user.o samples/bpf/execsnoop_user.c
```

- build `execsnoop`

```bash
  gcc -Wall -O2 -Wmissing-prototypes -Wstrict-prototypes \
	  -I./usr/include \
	  -I./tools/testing/selftests/bpf/ \
	  -I./tools/lib/ \
	  -I./tools/include \
	  -I./tools/perf \
	  -DHAVE_ATTR_TEST=0 \
	  -o samples/bpf/execsnoop \
	  samples/bpf/bpf_load.o samples/bpf/execsnoop_user.o \
	  tools/testing/selftests/bpf/trace_helpers.o tools/lib/bpf/libbpf.a \
	  -lelf -lz
```

## ARM64

```bash
# if cross compile
export ARCH=arm64
export CROSS_COMPILE=aarch64-linux-gnu-
```

The recommend way is to build in [ARM Docker Containers](https://www.stereolabs.com/docs/docker/building-arm-container-on-x86/). see `arm_docker.md`

- make

```bash
# clean
make mrproper
make -C tools clean
make -C samples/bpf clean
# make
make defconfig && make prepare
make headers_install -j8
# build samples/bpf
make samples/bpf -j8
# build bpftool
make tools/bpf -j8
```

- detail build `execsnoop_kern.o`

  note `-g` may not needed

```bash
clang  -nostdinc \
	-isystem /usr/lib/gcc/aarch64-linux-gnu/9/include \
	-I./arch/arm64/include -I./arch/arm64/include/generated  \
	-I./include -I./arch/arm64/include/uapi \
	-I./arch/arm64/include/generated/uapi \
	-I./include/uapi \
	-I./include/generated/uapi \
	-include ./include/linux/kconfig.h \
    -I./samples/bpf \
    -I./tools/testing/selftests/bpf/ \
    -I./tools/lib/ \
    -include asm_goto_workaround.h \
    -D__KERNEL__ -D__BPF_TRACING__ -Wno-unused-value -Wno-pointer-sign \
    -D__TARGET_ARCH_arm64 -Wno-compare-distinct-pointer-types \
    -Wno-gnu-variable-sized-type-not-at-end \
    -Wno-address-of-packed-member -Wno-tautological-compare \
    -Wno-unknown-warning-option  \
    -fno-stack-protector \
    -O2 -emit-llvm -c samples/bpf/execsnoop_kern.c \
    -o -| llc -march=bpf  -filetype=obj -o samples/bpf/execsnoop_kern.o
```

- generate

```
bpftool gen skeleton execsnoop_kern.o > aarch64/execsnoop_kern_skel.h
```



http://www.redfelineninja.org.uk/daniel/2018/02/running-an-iso-installer-image-for-arm64-aarch64-using-qemu-and-kvm/

```
qemu-system-aarch64 -cpu cortex-a53 -M virt -m 2048 -nographic \
-drive if=pflash,format=raw,file=QEMU_EFI.img \
-drive if=virtio,format=raw,file=ubuntu-20.04-live-server-arm64.iso
```



## Some resources

- [A thorough introduction to eBPF](https://lwn.net/Articles/740157/)
- [Write eBPF program in pure C](http://terenceli.github.io/%E6%8A%80%E6%9C%AF/2020/01/18/ebpf-in-c)
