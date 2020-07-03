
## Prons and cons

- use stable execve tracepoint, so build once and should work everywhere
- need to build in kernel tree

## Build in kernel tree

- ready and config kernel tree

```bash
# kernel config
gunzip -c /proc/config.gz > .config
make oldconfig && make prepare
# install headers to ./usr/include
make headers_install -j8
# build bpf
make M=samples/bpf -j8
```

- put or link `execsnoop_kern.c` and `execsnoop_user.c` to *samples/bpf/*
- edit  *samples/bpf/makefile*

```makefile
# in samples/bpf/makefile
tprogs-y += execsnoop
execsnoop-objs := bpf_load.o execsnoop_user.o $(TRACE_HELPERS)
always-y += execsnoop_kern.o
```

## Run

```bash
cd samples/bpf
sudo bash -c "ulimit -l unlimited && ./execsnoop"
```

## Detail build command

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
	-fno-stack-protector -g \
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

## Some resources

- [A thorough introduction to eBPF](https://lwn.net/Articles/740157/)
- [Write eBPF program in pure C](http://terenceli.github.io/%E6%8A%80%E6%9C%AF/2020/01/18/ebpf-in-c)
