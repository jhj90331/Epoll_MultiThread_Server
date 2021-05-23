/*
 * headers.h
 *
 *  Created on: 2021. 4. 1.
 *      Author: hsr
 */

#ifndef HEADERS_H_
#define HEADERS_H_

#include <asm-generic/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdio>
#include <iostream>
#include <string>
#include <map>
#include <unordered_map>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <algorithm>
#include <pthread.h>
#include <future>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <set>
#include <memory>


#define LISTEN_BACKLOG 15
#define PORT 2021
#define DBPORT 1995

#define MAX_ID_SIZE 50
#define MAX_PW_SIZE 50


using namespace std;





#endif /* HEADERS_H_ */
