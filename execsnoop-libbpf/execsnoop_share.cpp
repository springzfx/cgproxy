// Based on execsnoop(8) from BCC by Brendan Gregg and others.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
extern "C" {
#define typeof(x) decltype(x)
#include "execsnoop.h"
#include "execsnoop.skel.h"
#include "trace_helpers.h"
}
#include "execsnoop_share.h"

#define PERF_BUFFER_PAGES   64
#define NSEC_PRECISION (NSEC_PER_SEC / 1000)
#define MAX_ARGS_KEY 259

namespace CGPROXY::EXECSNOOP {
	
function<int(int)> callback = NULL;
promise<void> status;

void handle_event(void *ctx, int cpu, void *data, __u32 data_sz) {

  	auto e = static_cast<event*>(data);
  	int pid = e->pid;
  	if (callback) callback(pid);

}

void handle_lost_events(void *ctx, int cpu, __u64 lost_cnt) {
	fprintf(stderr, "Lost %llu events on CPU #%d!\n", lost_cnt, cpu);
}

int execsnoop() {
	struct perf_buffer_opts pb_opts;
	struct perf_buffer *pb = NULL;
	struct execsnoop_bpf *obj;
	int err;

	// libbpf_set_print(libbpf_print_fn);

	err = bump_memlock_rlimit();
	if (err) {
		fprintf(stderr, "failed to increase rlimit: %d\n", err);
		return 1;
	}

	obj = execsnoop_bpf__open();
	if (!obj) {
		fprintf(stderr, "failed to open and/or load BPF object\n");
		return 1;
	}

	/* initialize global data (filtering options) */
	obj->rodata->ignore_failed = true;

	err = execsnoop_bpf__load(obj);
	if (err) {
		fprintf(stderr, "failed to load BPF object: %d\n", err);
		goto cleanup;
	}

	err = execsnoop_bpf__attach(obj);
	if (err) {
		fprintf(stderr, "failed to attach BPF programs\n");
		goto cleanup;
	}

	/* setup event callbacks */
	pb_opts.sample_cb = handle_event;
	pb_opts.lost_cb = handle_lost_events;
	pb = perf_buffer__new(bpf_map__fd(obj->maps.events), PERF_BUFFER_PAGES, &pb_opts);
	err = libbpf_get_error(pb);
	if (err) {
		pb = NULL;
		fprintf(stderr, "failed to open perf buffer: %d\n", err);
		goto cleanup;
	}

  status.set_value();

	/* main: poll */
	while ((err = perf_buffer__poll(pb, 100)) >= 0);
	printf("Error polling perf buffer: %d\n", err);

cleanup:
	perf_buffer__free(pb);
	execsnoop_bpf__destroy(obj);

	return err != 0;
}


void startThread(function<int(int)> c, promise<void> _status) {
  status = move(_status);
  callback = c;
  execsnoop();
}
}
