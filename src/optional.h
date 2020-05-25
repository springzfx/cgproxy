#ifndef OPTIONAL_H
#define OPTIONAL_H 1

#include <functional>
using namespace std;

namespace CGPROXY::EXESNOOP {

struct thread_arg {
  function<int(int)> handle_pid;
};
void *startThread(void *arg);

}

#endif