/*
 * mgServer.h
 *
 *  Created on: 2021. 4. 1.
 *      Author: hsr
 */

#ifndef MGSERVER_H_
#define MGSERVER_H_

#include <headers.h>
#include <packet.h>
#include <userManager.h>
#include <threadPool.h>

#define MAX_EVENTS 1024
#define MAX_DATA 3000

class mgServer
{
private:
	int m_error_check;
	int m_server_fd;
	struct sockaddr_in m_SockAddr;
	int m_epoll_fd;
	struct epoll_event m_events;
	struct epoll_event m_epoll_events[MAX_EVENTS];
	int m_event_count;
	int m_timeout = -1;

	userManager m_userManager;
	ThreadPool m_threadPool;
	string m_recvBuf;
	mutex m_recvMutex;
	char m_dataStr[MAX_DATA];
	queue< pair<int,PACKET> > m_packetQueue;
	mutex m_packetQueue_Mutex;
	PACKET m_tmpPacket;
	int m_queue_client_fd;

public:
	~mgServer();
	void mg_server_start();
	void mg_socket();
	void mg_bind();
	void mg_ctl();
	void mg_mainLoop();
	void mg_connect();
	void mg_threadPoolWork(int recved_Client_fd ,PACKET recvedPacket);

	void mg_queueThreadWork();
};

#endif /* MGSERVER_H_ */

















