
#include <signal.h>
#include <bpf/libbpf.h>
#include "bpf_load.h"

#define TASK_COMM_LEN 16
struct event {
	char comm[TASK_COMM_LEN];
	pid_t pid;
	pid_t tgid;
	pid_t ppid;
	uid_t uid;
};
static void print_bpf_output(void *ctx, int cpu, void *data, __u32 size) {
	struct event *e=data;
    printf("comm: %s, pid: %d, tgid: %d, ppid: %d, uid: %d\n",e->comm,e->pid,e->tgid,e->ppid,e->uid);
}

int main(int argc, char **argv) {
	struct perf_buffer_opts pb_opts = {};
	struct perf_buffer *pb;
	int ret;

	if (load_bpf_file("execsnoop_kern.o")!=0) {
		printf("%s", bpf_log_buf);
		return 1;
	}

	pb_opts.sample_cb = print_bpf_output;
	pb = perf_buffer__new(map_fd[1], 8, &pb_opts);
	ret = libbpf_get_error(pb);
	if (ret) {
		printf("failed to setup perf_buffer: %d\n", ret);
		return 1;
	}

	while ((ret = perf_buffer__poll(pb, -1)) >= 0) {}
	kill(0, SIGINT);
	return ret;
}
