/*
 * mgServer.cpp
 *
 *  Created on: 2021. 4. 1.
 *      Author: hsr
 */

#include <mgServer.h>


mgServer::~mgServer()
{
	cout << "--------------------------------" << endl;
	cout << "Server End" << endl;
	cout << endl;
}


void mgServer::mg_server_start()
{
	cout << "Server Start" << endl;

	m_threadPool.ThreadPoolStart(8);	// 스레드풀 (스레드 개수) 만큼 생성
	mg_socket();
	mg_bind();
	mg_ctl();
	mg_mainLoop();

}


void mgServer::mg_socket()
{
	// 소켓 생성
	m_server_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (m_server_fd < 0) {
		cout << "mg_socket() ERROR!" << endl;
		return;
	}

	// server_fd를 NONBLOCK FD로 설정함
	int flags = fcntl(m_server_fd, F_GETFL);
	flags |= O_NONBLOCK;

	if (fcntl(m_server_fd, F_SETFL, flags) < 0)
	{
		cout << "server_fd fcntl() error" << endl;
		return;
	};

	// 소켓 옵션 설정
	int option = true;
	m_error_check = setsockopt(m_server_fd, SOL_SOCKET, SO_REUSEADDR, &option,
			sizeof(option));
	if (m_error_check < 0) {
		cout << "setsockopt() error " << m_error_check << endl;
		close(m_server_fd);
		return;
	}

	// 소켓 속성 설정
	memset(&m_SockAddr, 0, sizeof(m_SockAddr));
	m_SockAddr.sin_family = AF_INET;
	m_SockAddr.sin_port = htons(1818);
	m_SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	// INADDR_ANY : 사용가능한 랜카드 IP 사용

}


void mgServer::mg_bind()
{
	m_error_check = bind(m_server_fd, (struct sockaddr*) &m_SockAddr,
			sizeof(m_SockAddr));

	if (m_error_check < 0)
	{
		printf("bind() error[%d]\n", m_error_check);
		close(m_server_fd);
		return;
	}

	// 리슨
	if (listen(m_server_fd, LISTEN_BACKLOG) < 0)
	{
		cout << "listen() error" << endl;
		close(m_server_fd);
		return;
	}
}


void mgServer::mg_ctl()
{
	// Epoll fd 생성
	m_epoll_fd = epoll_create(1024);    // size 만큼의 커널 폴링 공간을 만드는 함수
	if (m_epoll_fd < 0)
	{
		cout << "epoll_create() error" << endl;
		close(m_server_fd);
		return;
	}

	m_events.events = EPOLLIN;
	m_events.data.fd = m_server_fd;

	// epoll_ctl : epoll이 관심을 가져주기 바라는 FD와
	//			     그 FD에서 발생하는 event를 등록하는 인터페이스.
	// EPOLL_CTL_ADD : 관심있는 파일디스크립트 추가
	// EPOLL_CTL_MOD : 기존 파일 디스크립터를 수정
	// EPOLL_CTL_DEL : 기존 파일 디스크립터를 관심 목록에서 삭제
	if (epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_server_fd, &m_events) < 0)
	{
		cout << "epoll_ctl() error" << endl;
		close(m_server_fd);
		close(m_epoll_fd);
		return;
	}

}



void mgServer::mg_mainLoop()
{
	struct epoll_event epoll_events[MAX_EVENTS];
	int event_count;
	int timeout = -1;

	thread queueThread(&mgServer::mg_queueThreadWork, this);

	cout << "Main Loop Start" << endl;

	while (true)
	{
		event_count = epoll_wait(m_epoll_fd, epoll_events, MAX_EVENTS, timeout);

		if (event_count < 0)
		{
			printf("epoll_wait() error [%d]\n", event_count);
			return;
		}

		for (int i = 0; i < event_count; i++)
		{
			// Accept
			if (epoll_events[i].data.fd == m_server_fd)
			{
				mg_connect();
			}
			else
			{
				// epoll에 등록 된 클라이언트들의 send data 처리
				int str_len;
				int client_fd = epoll_events[i].data.fd;
				char data[MAX_DATA];
				// user &clientUser = m_userManager.find_user(client_fd);

				str_len = read(client_fd, &data, sizeof(data));

				if (str_len == 0)
				{
					// 클라이언트 접속 종료 요청
					printf("Client Disconnect [%d]\n", client_fd);
					m_userManager.del_user(client_fd);
					close(client_fd);
					epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
				}
				else
				{
					// 접속 종료 요청이 아닌 경우 요청의 내용에 따라 처리.
					// 21.04.08 TODO: 패킷 분리시 처리  - 아직 못함
					// sizeof(PACKET) = 1043
					// 여기서 data size < PACKET size 이면 while(read) 해버릴까?
					// -> read해서 EAGAIN일때까지 계속 read하기

					int offset = str_len;
					char all_recved[3000] = {0};
					memcpy(all_recved, data, str_len);
					//memcpy(all_recved + str_len, data, str_len);
					bool empty_Recv_Data = false;

					while(empty_Recv_Data == false)
					{
						str_len = read(client_fd, &data, sizeof(data));

						if (str_len > 0)
						{
							cout << "추가 RECV" << endl;
							memcpy(all_recved + offset, data, str_len);

							offset += str_len;

							cout << "offset : " << offset << endl;
							cout << "str_len : " << str_len << endl;
						}
						else if (str_len == -1)
						{
							if (errno == EAGAIN)
								empty_Recv_Data = true;
							else
								cout << "read Error" << endl;
						}
					}

					int packetOffset = 0;
					while(offset > 0)
					{
						PACKET *pack = (PACKET*) (all_recved + packetOffset);

						m_packetQueue_Mutex.lock();
						m_packetQueue.push(make_pair(client_fd, *pack));
						m_packetQueue_Mutex.unlock();

						offset -= sizeof(PACKET);
						packetOffset += sizeof(PACKET);
					}

				} // else
			} // else
		} // for
	} // while
} // mg_mainLoop()


void mgServer::mg_queueThreadWork()
{
	while(1)
	{
		if(m_packetQueue.empty() != true)
		{
			m_packetQueue_Mutex.lock();


			m_queue_client_fd = m_packetQueue.front().first;
			m_tmpPacket = m_packetQueue.front().second;
			m_packetQueue.pop();
			m_threadPool.EnqueueJob([this](){this->mg_threadPoolWork(m_queue_client_fd,m_tmpPacket);});

			m_packetQueue_Mutex.unlock();
		}

		sleep(0.0001);
	}
}


// 스레드풀에서 실행할 함수
void mgServer::mg_threadPoolWork(int client_fd ,PACKET recvedPacket)
{
	m_recvMutex.lock();

	user &recvUser = m_userManager.find_user(client_fd);

	switch(recvedPacket.body.cmd)
	{
	case EM_CMD_USER_CHAT_REQ:
	{
		cout << "recved from " << client_fd << " : " << recvedPacket.body.data << endl;

		string userChat = string(recvedPacket.body.data);

		cout << "ready ansPacket" << endl;
		recvUser.add_Log(userChat);


		// 답 패킷 전송
		PACKET ansPacket;
		ansPacket.body.cmd = EM_CMD_USER_CHAT_RESULT;
		strcpy(ansPacket.body.data, "Chat Log add");
		ansPacket.phead.datasize = strlen(ansPacket.body.data);

		if( send(client_fd, (char*) &ansPacket, sizeof(PACKET), 0) == -1)
		{
			perror("send");
		}

		break;
	} // EM_CMD_USER_CHAT_REQ

	case EM_CMD_USER_LOG_REQ:
	{
		PACKET ansPacket;
		ansPacket.body.cmd = EM_CMD_USER_LOG_RESULT;

		string userLog = recvUser.get_Log();

		strcpy(ansPacket.body.data, userLog.c_str());

		ansPacket.phead.datasize = strlen(ansPacket.body.data);

		if (send(client_fd, (char*) &ansPacket, sizeof(PACKET), 0) == -1)
		{
			perror("send");
		}

		break;
	} // EM_CMD_USER_LOG_REQ

	case EM_CMD_USER_LOG_DELETE_REQ:
	{
		// 로그 지우기
		if(recvUser.is_emptyLog() == false)
		{
			recvUser.clear_Log();
		}

		// 답 패킷 전송
		PACKET ansPacket;
		ansPacket.body.cmd = EM_CMD_USER_LOG_DELETE_RESULT;
		strcpy(ansPacket.body.data, "Chat Log deleted");
		ansPacket.phead.datasize = strlen(ansPacket.body.data);

		if (send(client_fd, (char*) &ansPacket, sizeof(PACKET), 0) == -1)
		{
			perror("send");
		}

		break;
	} // EM_CMD_USER_LOG_DELETE_REQ


		default:
		{
		break;
		}
	} // switch



	m_recvMutex.unlock();
} // mg_threadPoolWork


// 클라이언트가 새로 접속 했을 때의 처리
void mgServer::mg_connect()
{
	cout << "client connect " << endl;
	int client_fd;
	int client_len;
	struct sockaddr_in client_addr;

	printf("User Accept\n");
	client_len = sizeof(client_addr);
	client_fd = accept(m_server_fd, (struct sockaddr*) &client_addr,
			(socklen_t*) &client_len);

	// client fd Non-Blocking Socket으로 설정. Edge Trigger 사용하기 위해 설정.
	int flags = fcntl(client_fd, F_GETFL);
	flags |= O_NONBLOCK;
	if (fcntl(client_fd, F_SETFL, flags) < 0)
	{
		cout << "client_fd[" << client_fd << "] fcntl() error" << endl;
		return;
	}

	if (client_fd < 0)
	{
		cout << "accept() error [%d" << client_fd << "]" << endl;
		return;
	}

	// 클라이언트 fd, epoll 에 등록
	struct epoll_event events;
	events.events = EPOLLIN | EPOLLET | EPOLLEXCLUSIVE;
	events.data.fd = client_fd;

	if (epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, client_fd, &events) < 0)
	{
		cout << "client epoll_ctl() error" << endl;
		close(client_fd);
		return;
	}

	cout << "Client Connect [" << client_fd << "]" << endl;

	m_userManager.add_user(client_fd);
} // mg_connect























