#ifndef SOCKET_CLIENT_H
#define SOCKET_CLIENT_H

#include <stdlib.h>
#include <string>
using namespace std;

namespace CGPROXY::SOCKET {

void send(const char *msg, int &status);
void send(const string msg, int &status);

} // namespace CGPROXY::SOCKET
#endif