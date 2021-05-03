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

	m_threadPool.ThreadPoolStart(100);	// 스레드풀 (스레드 개수) 만큼 생성
	thread watcherThread(&userManager::is_alive_thread, &m_userManager);
	watcherThread.detach();
	mg_socket();
	mg_bind();
	mg_ctl();
	mg_mainLoop();

}



//******************************************************
// 소켓 기본 설정
//******************************************************
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

//******************************************************
// 메인 루프
//******************************************************
void mgServer::mg_mainLoop()
{
	struct epoll_event epoll_events[MAX_EVENTS];
	int event_count;
	int timeout = -1;

	cout << "Main Loop Start" << endl;

	//thread queueThread(&mgServer::mg_queueThreadWork, this);

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
				//  epoll_events[i].data.fd 값으로 유저 매니저 검색 -> user 클래스의 recvInUser 실행
				m_userManager.find_user(epoll_events[i].data.fd)->recvInUser();

			}

		} // else
	} // while
} // mg_mainLoop()



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

	if (m_threadPool.get_restThreadNum() == 0)
	{
		cout << "Client Limited!!!!" << endl;
		close(client_fd);
		return;
	}

	// 클라이언트 fd, epoll 에 등록
	struct epoll_event events;
	events.events = EPOLLIN | EPOLLET;
	events.data.fd = client_fd;

	if (epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, client_fd, &events) < 0)
	{
		cout << "client epoll_ctl() error" << endl;
		close(client_fd);
		return;
	}

	cout << "Client Connect [" << client_fd << "]" << endl;



	user* tmpUser = new user(client_fd);
	m_userManager.add_user(client_fd, tmpUser);

	// FIXED
	m_queue_client_fd = client_fd;
	m_threadPool.EnqueueJob([tmpUser](){tmpUser->userThreadWork();});

} // mg_connect























