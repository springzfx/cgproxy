#ifndef EXECSNOOP_HPP
#define EXECSNOOP_HPP 1

#include <functional>
#include <future>
#include <string>
using namespace std;

namespace CGPROXY::EXECSNOOP {

extern const string BPF_PROGRAM;
struct data_t;
extern function<int(int)> callback;
void handle_events(void *cb_cookie, void *data, int data_size);
int execsnoop();

extern "C" void startThread(function<int(int)> c, promise<void> _status);
// typedef void startThread_t(function<int(int)>, promise<void>);
using startThread_t=decltype(startThread);
startThread_t *_startThread; // only for dlsym()

} // namespace CGPROXY::EXECSNOOP
#endif