// #include "common.h"
#include "config.hpp"
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
using namespace std;
using namespace CGPROXY::CONFIG;

int main() {
  Config c;
  c.saveToFile("./config.json");
  return 0;
}