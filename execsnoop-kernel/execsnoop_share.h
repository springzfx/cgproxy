#ifndef EXECSNOOP_SHARE_HPP
#define EXECSNOOP_SHARE_HPP 1

#include <functional>
#include <future>
#include <string>
using namespace std;

namespace CGPROXY::EXECSNOOP {

extern "C" void startThread(function<int(int)> c, promise<void> _status);

// only for dlsym()
using startThread_t=decltype(startThread);
startThread_t *_startThread;

} // namespace CGPROXY::EXECSNOOP
#endif
