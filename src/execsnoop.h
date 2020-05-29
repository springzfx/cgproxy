#ifndef EXECSNOOP_HPP
#define EXECSNOOP_HPP 1

#include <functional>
#include <string>
using namespace std;

namespace CGPROXY::EXECSNOOP {

extern const string BPF_PROGRAM;
struct data_t;
extern function<int(int)> callback;
void handle_events(void *cb_cookie, void *data, int data_size);
int execsnoop();

struct thread_arg {
  function<int(int)> handle_pid;
};
extern "C" void *startThread(void *arg);
decltype(&startThread) _startThread; // only for dlsym()

} // namespace CGPROXY::EXECSNOOP
#endif