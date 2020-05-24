#ifndef EXECSNOOP_HPP
#define EXECSNOOP_HPP 1

#include <exception>
#include "bcc/BPF.h"
#include "common.h"
#include <bcc/libbpf.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <unistd.h>
using namespace std;

namespace CGPROXY::EXESNOOP {

const string BPF_PROGRAM = R"(
#include <linux/fs.h>
#include <linux/sched.h>
#include <uapi/linux/ptrace.h>

struct data_t {
    int pid;
};

BPF_PERF_OUTPUT(events);

int syscall_execve(struct pt_regs *ctx,
    const char __user *filename,
    const char __user *const __user *__argv,
    const char __user *const __user *__envp)
{
    struct data_t data = {};
    data.pid = bpf_get_current_pid_tgid();
    events.perf_submit(ctx, &data, sizeof(struct data_t));
    return 0;
}

int ret_syscall_execve(struct pt_regs *ctx){
    struct data_t data = {};
    data.pid = bpf_get_current_pid_tgid();
    int retval = PT_REGS_RC(ctx);
    if (retval==0)
      events.perf_submit(ctx, &data, sizeof(struct data_t));
    return 0;
}
)";

struct data_t {
  int pid;
};

function<int(int)> callback = NULL;

void handle_events(void *cb_cookie, void *data, int data_size) {
  auto event = static_cast<data_t *>(data);
  int pid = event->pid;

  if (callback) callback(pid);
}

int execsnoop() {
  debug("starting execsnoop");
  ebpf::BPF bpf;
  auto init_res = bpf.init(BPF_PROGRAM);
  if (init_res.code() != 0) {
    std::cerr << init_res.msg() << std::endl;
    return 1;
  }

  string execve_fnname = bpf.get_syscall_fnname("execve");
  // auto attach_res = bpf.attach_kprobe(execve_fnname, "syscall_execve");
  auto attach_res =
      bpf.attach_kprobe(execve_fnname, "ret_syscall_execve", 0, BPF_PROBE_RETURN);
  if (attach_res.code() != 0) {
    std::cerr << attach_res.msg() << std::endl;
    return 1;
  }

  auto open_res = bpf.open_perf_buffer("events", &handle_events);
  if (open_res.code() != 0) {
    std::cerr << open_res.msg() << std::endl;
    return 1;
  }

  if (bpf.free_bcc_memory()) {
    std::cerr << "Failed to free llvm/clang memory" << std::endl;
    return 1;
  }

  while (true) bpf.poll_perf_buffer("events");

  return 0;
}

struct thread_arg {
  function<int(int)> handle_pid;
};

void *startThread(void *arg) {
  thread_arg *p = (thread_arg *)arg;
  callback = p->handle_pid;
  try {
    execsnoop();
  } catch (exception &e) {
    error("bcc may not be installed, %s",e.what());
  }
  return (void *)0;
}
} // namespace CGPROXY::EXESNOOP
#endif