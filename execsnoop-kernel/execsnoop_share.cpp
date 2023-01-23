#include "execsnoop_share.h"

#include <errno.h>
#include <signal.h>
#include <bpf/libbpf.h>
#include <sys/resource.h>

#include <utility>

#if defined(__x86_64__)
	#include "x86_64/execsnoop_kern_skel.h"
#elif defined(__aarch64__)
	#include "aarch64/execsnoop_kern_skel.h"
#endif

namespace CGPROXY::EXECSNOOP {

#define PERF_BUFFER_PAGES 64
#define TASK_COMM_LEN 16
struct event {
	char comm[TASK_COMM_LEN];
	pid_t pid;
	pid_t tgid;
	pid_t ppid;
	uid_t uid;
};

std::function<int(int)> callback = nullptr;
std::promise<void> status;

static void handle_event(void *ctx, int cpu, void *data, __u32 size) {
  	auto *e = static_cast<event*>(data);
  	if (callback) callback(e->pid);
}

void handle_lost_events(void *ctx, int cpu, __u64 lost_cnt) {
	fprintf(stderr, "Lost %llu events on CPU #%d!\n", lost_cnt, cpu);
}

int bump_memlock_rlimit() {
	struct rlimit rlim_new = { RLIM_INFINITY, RLIM_INFINITY };
	return setrlimit(RLIMIT_MEMLOCK, &rlim_new);
}

int execsnoop() {
	struct perf_buffer_opts pb_opts = {};
	struct perf_buffer *pb;
	int err;
	bool notified=false;
	
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

main_loop:
	pb_opts.sample_cb = handle_event;
	pb_opts.lost_cb = handle_lost_events;
	pb = perf_buffer__new(bpf_map__fd(obj->maps.perf_events), PERF_BUFFER_PAGES, &pb_opts);
	err = libbpf_get_error(pb);
	if (err) {
		printf("failed to setup perf_buffer: %d\n", err);
		return 1;
	}

	// notify
  	if (!notified) {status.set_value(); notified=true;}

	while ((err = perf_buffer__poll(pb, -1)) >= 0) {}
	perf_buffer__free(pb);
	/* handle Interrupted system call when sleep */
	if (err == -EINTR) goto main_loop;

	perror("perf_buffer__poll");
	kill(0, SIGINT);
	return err;
}

void startThread(std::function<int(int)> c, std::promise<void> _status) {
  status = std::move(_status);
  callback = std::move(c);
  execsnoop();
}

}
