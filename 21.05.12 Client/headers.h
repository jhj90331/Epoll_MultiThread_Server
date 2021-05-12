#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <sys/ioctl.h>
#include <thread>
#include <mutex>
#include <algorithm>
#include <map>
#include <unordered_map>
#include <vector>

#define PORT 2021
#define DBPORT 1995

using namespace std;
