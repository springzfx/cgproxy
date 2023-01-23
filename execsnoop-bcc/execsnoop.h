#pragma once

#include <functional>
#include <future>
#include <string>

namespace CGPROXY::EXECSNOOP {

extern const std::string BPF_PROGRAM;
struct data_t;
extern std::function<int(int)> callback;
void handle_events(void *cb_cookie, void *data, int data_size);
int execsnoop();

extern "C" void startThread(std::function<int(int)> c, std::promise<void> _status);
// typedef void startThread_t(std::function<int(int)>, std::promise<void>);
using startThread_t=decltype(startThread);
startThread_t *_startThread; // only for dlsym()

} // namespace CGPROXY::EXECSNOOP
