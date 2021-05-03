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


bool user::recvInUser()
{
	int str_len = 0;
	char data[USER_MAX_DATA];

	str_len = read(m_client_fd, &data, sizeof(data));

	if(str_len == 0)
	{
		// 접속 종료
		cout << " USER STRLEN 0" << endl;
		m_userLive = false;
	}
	else
	{
		m_ringBuffer.enqueue(data, str_len);
		bool empty_Recv_Data = false;

		// recv 다 받기
		while (empty_Recv_Data == false)
		{
			str_len = read(m_client_fd, &data, sizeof(data));

			if (str_len > 0)
			{
				cout << "추가 RECV" << endl;
				m_ringBuffer.enqueue(data, str_len);
			}
			else if (str_len == -1)
			{
				if (errno == EAGAIN)
					empty_Recv_Data = true;
				else
					cout << "read Error" << endl;
			}
		}

		while (m_ringBuffer.is_empty() == false)
		{
			char headBuf[sizeof(PACKET) + 1] = {0};
			HEADER head;
			cout << "CURRENT RING SIZE : " << m_ringBuffer.get_curSize() << endl;
			m_ringBuffer.dequeue(headBuf, sizeof(HEADER));
			memcpy(&head, headBuf, sizeof(HEADER));

			if (strcmp(head.head, "AA11") != 0)
			{
				cout << "HEADER HEAD IS NOT AA11" << endl;
			}

			char bodyBuf[sizeof(BODY) + 1] = {0};
			m_ringBuffer.dequeue(bodyBuf, head.bodysize);
			memcpy(headBuf + sizeof(HEADER), bodyBuf, head.bodysize);

			char tailBuf[sizeof(TAIL) + 1] = {0};
			m_ringBuffer.dequeue(tailBuf, sizeof(TAIL));

			if (strncmp(tailBuf, "11AA", 5) != 0)
			{
				cout << "TAIL IS NOT 11AA" << endl;
			}

			cout << "PACKET HEADER + BODY SIZE : " << sizeof(HEADER) + head.bodysize << endl;
			PACKET pack;
			decodePacket(headBuf, pack);

			m_pkQueue_Mutex.lock();
			m_pkQueue.push(pack);
			m_pkQueue_Mutex.unlock();
		}


	}

	return false;
}


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
















