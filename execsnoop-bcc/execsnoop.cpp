#include "execsnoop.h"
#include "bcc/BPF.h"
#include "common.h"
#include <bcc/libbpf.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <unistd.h>
using namespace std;

namespace CGPROXY::EXECSNOOP {

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
promise<void> status;

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
    error("bpf init failed, maybe linux-headers not installed");
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

  status.set_value();

  while (true) bpf.poll_perf_buffer("events");

  return 0;
}

void startThread(function<int(int)> c, promise<void> _status) {
  status = move(_status);
  callback = c;
  execsnoop();
}

} // namespace CGPROXY::EXECSNOOP