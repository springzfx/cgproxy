#include <signal.h>
#include <bpf/libbpf.h>
#include <sys/resource.h>

#if defined(__x86_64__)
	#include "x86_64/execsnoop_kern_skel.h"
#elif defined(__aarch64__)
	#include "aarch64/execsnoop_kern_skel.h"
#endif

#define TASK_COMM_LEN 16
struct event {
	char comm[TASK_COMM_LEN];
	pid_t pid;
	pid_t tgid;
	pid_t ppid;
	uid_t uid;
};

#define PERF_BUFFER_PAGES   64

static void print_bpf_output(void *ctx, int cpu, void *data, __u32 size) {
	struct event *e=data;
    printf("comm: %s, pid: %d, tgid: %d, ppid: %d, uid: %d\n",e->comm,e->pid,e->tgid,e->ppid,e->uid);
}

int bump_memlock_rlimit(void)
{
	struct rlimit rlim_new = {
		.rlim_cur	= RLIM_INFINITY,
		.rlim_max	= RLIM_INFINITY,
	};

	return setrlimit(RLIMIT_MEMLOCK, &rlim_new);
}


int main(int argc, char **argv) {
	struct perf_buffer_opts pb_opts = {};
	struct perf_buffer *pb;
	int err;
	
	err = bump_memlock_rlimit();
	if (err) {
		fprintf(stderr, "failed to increase rlimit: %d\n", err);
		return 1;
	}
	
	struct execsnoop_kern *obj=execsnoop_kern__open_and_load();
	if (!obj) {
		fprintf(stderr, "failed to open and/or load BPF object\n");
		return 1;
	}

	err = execsnoop_kern__attach(obj);
	if (err) {
		fprintf(stderr, "failed to attach BPF programs\n");
		return err;
	}

	pb_opts.sample_cb = print_bpf_output;
	pb = perf_buffer__new(bpf_map__fd(obj->maps.perf_events), PERF_BUFFER_PAGES, &pb_opts);
	err = libbpf_get_error(pb);
	if (err) {
		printf("failed to setup perf_buffer: %d\n", err);
		return 1;
	}

	while ((err = perf_buffer__poll(pb, -1)) >= 0) {}
	kill(0, SIGINT);
	return err;
}
