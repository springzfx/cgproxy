
## Resource

- [libbpf](https://github.com/libbpf/libbpf)
- [libbpf-tools](https://github.com/iovisor/bcc/tree/master/libbpf-tools) examples use libbpf
- [bpftool](https://www.archlinux.org/packages/community/x86_64/bpf/) to generate skeleton and dump btf

## Prons

- BPF CO-RE (Compile Once – Run Everywhere)
- small memory usage

## Cons

- `vmlinux.h`does not contain `#define` etc. And often causes confilct with other headers to cause redifinition error
- comment in code for different types is gone
- need kernel built with `CONFIG_DEBUG_INFO_BTF=y`, while ubuntu 20.04 still not

## Build

- see *makefile*