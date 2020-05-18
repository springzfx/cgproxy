#include "cgproxy.hpp"
#include "cgproxyd.hpp"

bool as_cgproxyd = false;
void processArgs(const int argc, char *argv[]) {
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--daemon") == 0) { as_cgproxyd = true; }
    if (argv[i][0] != '-') { break; }
  }
}

int main(int argc, char *argv[]) {
  processArgs(argc, argv);
  if (as_cgproxyd) ::CGPROXY::CGPROXYD::main(argc, argv);
  else
    ::CGPROXY::CGPROXY::main(argc, argv);
}