# SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause)
OUTPUT := $(abspath build)
CLANG ?= clang
CFLAGS ?= -g -O2 -Wall
LLVM_STRIP ?= llvm-strip
BPFTOOL ?= bpftool
LIBBPF_DIR := $(abspath libbpf/src)
LIBBPF_OBJ := $(OUTPUT)/usr/lib64/libbpf.a
LIBBPF_H := $(OUTPUT)/usr/include
INCLUDES := -I$(OUTPUT) -I$(LIBBPF_H)
ARCH := $(shell uname -m | sed 's/x86_64/x86/')

APPS = execsnoop
LIBSO = libexecsnoop.so

.PHONY: all
all: $(APPS) $(LIBSO)

ifeq ($(V),1)
Q =
msg =
else
Q = @
msg = @printf '  %-8s %s%s\n' "$(1)" "$(notdir $(2))" "$(if $(3), $(3))";
MAKEFLAGS += --no-print-directory
endif

COMMON_OBJ = \
	$(OUTPUT)/trace_helpers.o \
	$(OUTPUT)/syscall_helpers.o \
	$(OUTPUT)/errno_helpers.o \


.PHONY: install
install: $(LIBSO)
	install -D $(LIBSO) -t $(DESTDIR)/usr/lib/cgproxy/

.PHONY: clean
clean:
	$(call msg,CLEAN)
	$(Q)rm -rf $(OUTPUT) $(APPS) $(LIBSO)

$(OUTPUT) $(OUTPUT)/libbpf:
	$(call msg,MKDIR,$@)
	$(Q)mkdir -p $@

$(APPS): %: $(OUTPUT)/%.o $(LIBBPF_OBJ) $(COMMON_OBJ) | $(OUTPUT)
	$(call msg,BINARY,$@)
	$(Q)$(CC) $(CFLAGS) $^ -lelf -lz -o $@

$(patsubst %,$(OUTPUT)/%.o,$(APPS)): %.o: %.skel.h

$(OUTPUT)/%.o: %.c $(wildcard %.h) | $(OUTPUT)
	$(call msg,CC,$@)
	$(Q)$(CC) $(CFLAGS) $(INCLUDES) -fPIC -c $(filter %.c,$^) -o $@

$(OUTPUT)/%.skel.h: $(OUTPUT)/%.bpf.o | $(OUTPUT)
	$(call msg,GEN-SKEL,$@)
	$(Q)$(BPFTOOL) gen skeleton $< > $@

$(OUTPUT)/%.bpf.o: %.bpf.c $(LIBBPF_OBJ) $(wildcard %.h) vmlinux.h | $(OUTPUT)
	$(call msg,BPF,$@)
	$(Q)$(CLANG) -g -O2 -target bpf -D__TARGET_ARCH_$(ARCH)		      \
		     $(INCLUDES) -c $(filter %.c,$^) -o $@ &&		      \
	$(LLVM_STRIP) -g $@

vmlinux.h:
	$(Q)$(BPFTOOL) btf dump file /sys/kernel/btf/vmlinux format c > vmlinux.h

$(LIBBPF_OBJ):
	$(Q)cd $(LIBBPF_DIR) && make DESTDIR=${OUTPUT} prefix=$(OUTPUT) CPPFLAGS="-fPIC" install install_headers

# patch: | $(LIBBPF_DIR)/Makefile
# 	@echo "patching libbpf-fPIC.patch"
# 	$(Q)cd libbpf/src && patch -p1 --forward -i ../../libbpf-fPIC.patch || true

# build libexecsnoop.so
$(LIBSO): execsnoop_share.cpp $(OUTPUT)/execsnoop.skel.h $(LIBBPF_OBJ) $(COMMON_OBJ) | $(OUTPUT) 
	$(call msg,LIB,$@)
	$(Q)$(CXX) -std=c++17 $(CFLAGS) $(INCLUDES) -fPIC $< $(COMMON_OBJ) -shared -lelf -lz -Wl,--no-whole-archive,--no-undefined $(LIBBPF_OBJ) -o $@


# delete failed targets
.DELETE_ON_ERROR:
# keep intermediate (.skel.h, .bpf.o, etc) targets
.SECONDARY:

