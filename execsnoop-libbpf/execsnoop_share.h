#ifndef EXECSNOOP_SHARE_HPP
#define EXECSNOOP_SHARE_HPP 1

#include <functional>
#include <future>
#include <string>

namespace CGPROXY::EXECSNOOP {

extern "C" void startThread(std::function<int(int)> c, std::promise<void> _status);

#ifdef BUIlD_EXECSNOOP_DL
// only for dlsym()
using startThread_t=decltype(startThread);
startThread_t *_startThread;
#endif

} // namespace CGPROXY::EXECSNOOP
#endif
