/*
 * user.cpp
 *
 *  Created on: 2021. 4. 2.
 *      Author: hsr
 */

#include <user.h>

user::user(int fd)
{
	m_client_fd = fd;
	m_userLive = true;
	m_threadWorking = false;
	//m_log.clear();
}

user::~user()
{

}

void user::set_fd(int fd)
{
	m_client_fd = fd;
}

int user::get_fd()
{
	return m_client_fd;
}

bool user::is_emptyLog()
{
	if(m_log.empty() == true)
		return true;
	else
		return false;
}

void user::clear_Log()
{
	if(m_log.empty() != true)
		m_log.clear();
}

string user::get_Log()
{
	string res;
	int size = m_log.size();

	for(int i = 0; i < size; i++)
	{
		res += m_log[i];
		res.push_back('\n');
	}
	return res;
}

void user::add_Log(string str)
{
	m_log.push_back(str);
	if(m_log.size() >= 20)
	{
		m_log.erase(m_log.begin());
	}
}

// USER 클래스에서 RECV
bool user::recvInUser()
{
	int str_len = 0;
	char data[USER_MAX_DATA];
	bool empty_Recv_Data = false;
	int cal_bodySize = 0; // end offset - sizeof(header)

	str_len = read(m_client_fd, &data, sizeof(data));

	if(str_len == 0)
	{
		// 접속 종료 요청
		cout << " USER STRLEN 0" << endl;
		m_userLive = false;
	}
	else
	{
		if (m_ringBuffer.enqueue(data, str_len) == false)
		{
			// 링 버퍼에 recv한 데이터가 들어갈 자리가 없는 경우
			cout << "CANNOT ENQUEUE" << endl;
			m_userLive = false;
			return false;
		}

		// recv 다 받기
		while (empty_Recv_Data == false)
		{
			str_len = read(m_client_fd, &data, sizeof(data));

			if (str_len > 0)
			{
				cout << "추가 RECV" << endl;
				if(m_ringBuffer.enqueue(data, str_len) == false)
				{
					// 링 버퍼에 recv한 데이터가 들어갈 자리가 없는 경우
					cout << "CANNOT ENQUEUE SECOND" << endl;
					m_userLive = false;
					break;
				}
			}
			else if (str_len == -1)
			{
				if (errno == EAGAIN) // recv 할 데이터가 더 이상 없음
					empty_Recv_Data = true;
				else
				{
					// RECV 오류 발생
					cout << "read Error" << endl;
					m_userLive = false;
					break;
				}
			}

			usleep(1000);
		}

		while (m_ringBuffer.is_empty() == false)
		{
			char headBuf[sizeof(PACKET) + 1] = {0};
			HEADER head;
			cout << "CURRENT RING SIZE : " << m_ringBuffer.get_curSize() << endl;

			if((unsigned)m_ringBuffer.get_curSize() < sizeof(HEADER)) // 버퍼안에 헤더 사이즈보다 적게 데이터가 있을 경우
			{
				cout << "NOT ENOUGH TO MAKE HEADER" << endl;
				// m_ringBuffer.cBuf_flush();	// 버퍼 비우고 종료
				break;
			}
			else
			{
				char headStr[5] = {"AA11"};
				char tailStr[5] = {"11AA"};

				int start_Offset = m_ringBuffer.cBuf_find(headStr); //

				if(start_Offset > 0) // 버퍼 안에 AA11이 있지만 앞에 다른 문자열이 있을 경우
				{
					cout << "SomeThing Blocking Before AA11" << endl;
					m_ringBuffer.pop(start_Offset);	// AA11 앞의 문자열 제거
					cout << "POPED RING BUF SIZE : " << m_ringBuffer.get_curSize() << endl;
				}
				else if(start_Offset < 0) // 버퍼 안에 AA11이 없을 경우
				{
					cout << "NO AA11 IN BUFFER" << endl;
					break;
				}

				// AA11 확인 후 버퍼에 11AA 확인
				int end_Offset = m_ringBuffer.cBuf_find(tailStr);

				if (end_Offset < 0)
				{
					// 버퍼 안에 11AA가 없을 경우
					cout << "NO TAIL IN BUFFER -> KEEP RECV" << endl;
					break;
				}
				else if(end_Offset - sizeof(HEADER) > MAX_BODY_SIZE)
				{
					// 계산된 BODY 사이즈가 최대 BODY 데이터 사이즈보다 클 경우
					cout << "PACKET BODY DATA TOO BIG" << endl;
					m_ringBuffer.pop(end_Offset + sizeof(TAIL)); // 보디 데이터가 너무 크면 그 패킷 POP
					break;
				}

				cal_bodySize = end_Offset - sizeof(HEADER); // AA11, 11AA 사이의 크기
			} // else

			// 링 버퍼에서 HEAD 꺼내기
			m_ringBuffer.dequeue(headBuf, sizeof(HEADER));
			memcpy(&head, headBuf, sizeof(HEADER));

			// head AA11 검사
			if (strcmp(head.head, "AA11") != 0)
			{
				cout << "HEADER HEAD IS NOT AA11" << endl;
				m_userLive = false;
				break;
			}

			// 헤드에 명시된 body사이즈와 실제 body사이즈가 다를 경우
			if(head.bodysize != cal_bodySize)
			{
				cout << "BODYSIZE NOT MATCHED" << endl;
				m_userLive = false;
				break;
			}


			// 링 버퍼에서 BODY 꺼내기
			char bodyBuf[sizeof(BODY) + 1] = {0};
			m_ringBuffer.dequeue(bodyBuf, head.bodysize);
			memcpy(headBuf + sizeof(HEADER), bodyBuf, head.bodysize);

			// 링 버퍼에서 TAIL 꺼내기
			char tailBuf[sizeof(TAIL) + 1] = {0};
			m_ringBuffer.dequeue(tailBuf, sizeof(TAIL));

			// TAIL 11AA 검사
			if (strncmp(tailBuf, "11AA", 5) != 0)
			{
				cout << "TAIL IS NOT 11AA" << endl;
				m_userLive = false;
				break;
			}

			cout << "PACKET HEADER + BODY SIZE : " << sizeof(HEADER) + head.bodysize << endl;

			// 받은 데이터로 패킷 조립
			PACKET pack;
			decodePacket(headBuf, pack);

			// 패킷 큐에 push
			m_pkQueue_Mutex.lock();
			m_pkQueue.push(pack);
			m_pkQueue_Mutex.unlock();
		} // while


		return true;
	} // else

	cout << "RECV IN USER FALSE RETURN" << endl;
	return false;
} // recvInUser


void user::userThreadWork()
{
	m_threadWorking = true;
	while(m_userLive)
	{
		if(m_pkQueue.empty() == false)
		{
			// 패킷 처리
			m_pkQueue_Mutex.lock();
			PACKET recvedPacket = m_pkQueue.front();
			m_pkQueue.pop();
			m_pkQueue_Mutex.unlock();

			cout << "Client [" << m_client_fd << "]" << "recv Packet" << endl;

			switch(recvedPacket.body.cmd)
			{
			case EM_CMD_USER_CHAT_REQ:
			{
				string userChat = string(recvedPacket.body.data);

				add_Log(userChat);

				// 답 패킷
				PACKET ansPacket;
				char tmpArr[50];
				strcpy(tmpArr, "Chat Log Add");

				makePacket(EM_CMD_USER_CHAT_RESULT, tmpArr, ansPacket);

				if ( send( m_client_fd, (char*) &ansPacket, sizeof(PACKET), 0 ) == -1 )
				{
					perror("send");
				}
				break;
			}

			case EM_CMD_USER_LOG_REQ:
			{
				string userLog = get_Log();
				char logTmp[1000];
				strncpy(logTmp, userLog.c_str(), userLog.length() + 1);

				// 답 패킷
				PACKET ansPacket;

				makePacket(EM_CMD_USER_LOG_RESULT, logTmp, ansPacket);

				if ( send( m_client_fd, (char*) &ansPacket, sizeof(PACKET), 0 )	== -1 )
				{
					perror("send");
				}

				break;
			}

			case EM_CMD_USER_LOG_DELETE_REQ:
			{
				if(is_emptyLog() == false)
				{
					clear_Log();
				}

				// 답 패킷
				PACKET ansPacket;
				ansPacket.body.cmd = EM_CMD_USER_LOG_DELETE_RESULT;
				char tmpArr[50];
				strcpy(tmpArr, "Chat Log Deleted");

				makePacket(EM_CMD_USER_LOG_DELETE_RESULT, tmpArr, ansPacket);

				if ( send( m_client_fd, (char*) &ansPacket, sizeof(PACKET), 0) == -1 )
				{
					perror("send");
				}

				break;
			}

			default:
			{
				break;
			}

			} // switch
		} // if

	usleep(1000);

	} // while

	cout << "userThread OVER " << endl;
	m_threadWorking = false;
}



bool user::is_alive()
{
	//cout << "user ALIVE IN" << endl;
	if(m_userLive == false && m_threadWorking == false)
	{
		//cout << "USER DEAD" << endl;
		return false;
	}
	else
		return true;
}
















