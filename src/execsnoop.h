#ifndef EXECSNOOP_HPP
#define EXECSNOOP_HPP 1

#include <functional>
#include <string>
using namespace std;

namespace CGPROXY::EXESNOOP {

extern const string BPF_PROGRAM;
struct data_t;
extern function<int(int)> callback;
void handle_events(void *cb_cookie, void *data, int data_size);
int execsnoop(); 

} // namespace CGPROXY::EXESNOOP
#endif