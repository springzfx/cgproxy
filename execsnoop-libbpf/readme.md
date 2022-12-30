generate `vmlinux.h`

```shell
bpftool btf dump file /sys/kernel/btf/vmlinux format c > vmlinux.h
```

compiled into BPF ELF file

```shell
clang -O2 -g -target bpf -c execsnoop.bpf.c -o execsnoop.bpf.o
```

generate BPF skeleton .skel.h

```shell
bpftool gen skeleton execsnoop.bpf.o > execsnoop.skel.h
```

