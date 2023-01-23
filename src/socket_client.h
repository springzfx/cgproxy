#ifndef SOCKET_CLIENT_H
#define SOCKET_CLIENT_H

#include <stdlib.h>
#include <string>

namespace CGPROXY::SOCKET {

void send(const char *msg, int &status);
void send(const std::string &msg, int &status);

} // namespace CGPROXY::SOCKET
#endif
