#include "optional.h"
#include "common.h"
#include "execsnoop.h"

namespace CGPROXY::EXESNOOP {

void *startThread(void *arg) {
  thread_arg *p = (thread_arg *)arg;
  callback = p->handle_pid;
  execsnoop();
  return (void *)0;
}

}