#ifdef USE_VMLINUX
#include "vmlinux.h"
#else
#include "linux/sched.h"
#include <linux/ptrace.h>
#include <uapi/linux/bpf.h>
#endif
#include <linux/version.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>

#define TASK_COMM_LEN 16
struct event {
	char comm[TASK_COMM_LEN];
	pid_t pid;
	pid_t tgid;
	pid_t ppid;
	uid_t uid;
};

/* /sys/kernel/debug/tracing/events/syscalls/sys_enter_execve/format */
struct syscalls_enter_execve_args {
	__u64 unused;
	int syscall_nr;
	const char filename_ptr;
    const char *const * argv;
    const char *const * envp;
};

/* /sys/kernel/debug/tracing/events/syscalls/sys_exit_execve/format */
struct syscalls_exit_execve_args {
	__u64 unused;
	int syscall_nr;
	long ret;
};

struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __uint(max_entries, 10240);
    __type(key, pid_t);
    __type(value, struct event);
} records SEC(".maps");

struct {
    __uint(type, BPF_MAP_TYPE_PERF_EVENT_ARRAY);
    __uint(max_entries, 128);
    __type(key, u32);
    __type(value, u32);
} perf_events SEC(".maps");

SEC("tracepoint/syscalls/sys_enter_execve")
int syscall_enter_execve(struct syscalls_enter_execve_args *ctx){
    pid_t pid, tgid; uid_t uid;
	struct event *event;
	struct task_struct *task, *task_p;

    u64 id = bpf_get_current_pid_tgid();
	pid = (pid_t)id;
	tgid = id >> 32;
	uid = (u32)bpf_get_current_uid_gid();

	struct event empty_event={};
	if (bpf_map_update_elem(&records, &pid, &empty_event, BPF_NOEXIST)!=0) return 0;
	event = bpf_map_lookup_elem(&records, &pid);
	if (!event) return 0;

	event->pid = pid;
	event->tgid = tgid;
	event->uid = uid;

	/* ppid is not reliable here, normal in arch, but become 0 in ubuntu 20.04 */
	task = (struct task_struct*)bpf_get_current_task();
	bpf_probe_read(&task_p, sizeof(struct task_struct*),&task->real_parent);
	bpf_probe_read(&event->ppid,sizeof(pid_t),&task_p->tgid);

    return 0;
}

SEC("tracepoint/syscalls/sys_exit_execve")
int syscall_exit_execve(struct syscalls_exit_execve_args *ctx){
	pid_t pid;
	struct event *event;

	pid = (pid_t)bpf_get_current_pid_tgid();
	event = bpf_map_lookup_elem(&records,&pid);
	if (!event) return 0;
	if (ctx->ret < 0) goto cleanup;

	/* get comm */
	bpf_get_current_comm(&event->comm,sizeof(event->comm));
	bpf_perf_event_output(ctx, &perf_events, BPF_F_CURRENT_CPU, event, sizeof(struct event));
cleanup:
	bpf_map_delete_elem(&records, &pid);
	return 0;
}

char _license[] SEC("license") = "GPL";
u32 _version SEC("version") = LINUX_VERSION_CODE;
