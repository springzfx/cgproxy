#include <signal.h>
#include <bpf/libbpf.h>
#include <sys/resource.h>
#include "execsnoop.skel.h"

#define TASK_COMM_LEN 16
struct event {
	char comm[TASK_COMM_LEN];
	pid_t pid;
	pid_t tgid;
	pid_t ppid;
	uid_t uid;
};

#define PERF_BUFFER_PAGES   64

static void handle_event(void *ctx, int cpu, void *data, __u32 size) {
  	auto e = static_cast<event*>(data);
    printf("comm: %s, pid: %d, tgid: %d, ppid: %d, uid: %d\n",e->comm,e->pid,e->tgid,e->ppid,e->uid);
}

void handle_lost_events(void *ctx, int cpu, __u64 lost_cnt) {
	fprintf(stderr, "Lost %llu events on CPU #%d!\n", lost_cnt, cpu);
}

int bump_memlock_rlimit(void) {
	struct rlimit rlim_new = { RLIM_INFINITY, RLIM_INFINITY };
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
	
	struct execsnoop_bpf *obj=execsnoop_bpf__open_and_load();
	if (!obj) {
		fprintf(stderr, "failed to open and/or load BPF object\n");
		return 1;
	}

	err = execsnoop_bpf__attach(obj);
	if (err) {
		fprintf(stderr, "failed to attach BPF programs\n");
		return err;
	}

	#if LIBBPF_MAJOR_VERSION == 0
	pb_opts.sample_cb = handle_event;
	pb_opts.lost_cb = handle_lost_events;
	pb = perf_buffer__new(bpf_map__fd(obj->maps.perf_events), PERF_BUFFER_PAGES, &pb_opts);
	#else
	pb_opts.sz = sizeof(pb_opts);
	pb = perf_buffer__new(bpf_map__fd(obj->maps.perf_events), PERF_BUFFER_PAGES, handle_event, handle_lost_events, nullptr, &pb_opts);
	#endif
	err = libbpf_get_error(pb);
	if (err) {
		printf("failed to setup perf_buffer: %d\n", err);
		return 1;
	}

	while ((err = perf_buffer__poll(pb, -1)) >= 0) {}
	kill(0, SIGINT);
	return err;
}
