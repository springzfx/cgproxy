#ifndef EXECSNOOP_SHARE_HPP
#define EXECSNOOP_SHARE_HPP 1

#include <functional>
#include <future>
#include <string>
using namespace std;

namespace CGPROXY::EXECSNOOP {
extern "C" void startThread(function<int(int)> c, promise<void> _status);
// typedef void startThread_t(function<int(int)>, promise<void>);
using startThread_t=decltype(startThread);
startThread_t *_startThread; // only for dlsym()

} // namespace CGPROXY::EXECSNOOP
#endif
