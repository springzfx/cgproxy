
## Prons and cons

- use stable execve tracepoint, so build once and should work everywhere
- build in kernel tree or build with VMLINUX



## Build `execsnoop_kern.o`

### M1: Build in kernel tree

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
make M=samples/bpf -j8
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


**Detail build command**

using `make V=1 M=samples/bpf | tee -a log.txt` to get and filter following command

- build `execsnoop_kern.o`


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
	-o - | llc -march=bpf -filetype=obj -o samples/bpf/execsnoop_kern.o
```



### M2: Build with VMLINUX

- get `vmlinux.h`

```
bpftool btf dump file /sys/kernel/btf/vmlinux format c > vmlinux.h
```

- compile

  note `-g` is needed if with BPF CO-RE

```
clang -O2 -target bpf -DUSE_VMLINUX -c execsnoop_kern.c -o execsnoop_kern.o
```

## Generate `execsnoop_kern_skel.h`

- generate `execsnoop_kern_skel.h`

```
bpftool gen skeleton execsnoop_kern.o > execsnoop_kern_skel.h
```

- build execsnoop

```
gcc -Wall -O2 execsnoop_user_1.c -o execsnoop -lbpf
```



## Multiarch build

- Cross compile, fast, but library link can be mess
  - aarch64-linux-gnu-gcc
- Emulation, the easist, but with perfomance cost
  - qemu-user-static + binfmt-qemu-static + docker/chroot
- see `arm64.md` to see how to setup



```bash
# if cross compile
export ARCH=arm64
export CROSS_COMPILE=aarch64-linux-gnu-
export SYSROOT=/home/fancy/workspace-xps/linux/ArchLinuxARM-aarch64-latest
export C_INCLUDE_PATH=$SYSROOT/usr/include
```

-  edit `tools/lib/bpf/makefile` #192 to:

```makefile
$(OUTPUT)libbpf.so.$(LIBBPF_VERSION): $(BPF_IN_SHARED)
     $(QUIET_LINK)$(CC) $(CFLAGS) $(LDFLAGS)
```

- make

```bash
# clean
make mrproper
make clean
make -C tools clean
make -C samples/bpf clean
# make
make defconfig && make prepare
make headers_install -j8
# build samples/bpf
make M=samples/bpf -j8
# build bpftool
make tools/bpf -j8
```

- detail build `execsnoop_kern.o`


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



## Refer

- [A thorough introduction to eBPF](https://lwn.net/Articles/740157/)

