/*
 * semiDB.cpp
 *
 *  Created on: 2021. 5. 10.
 *      Author: hsr
 */

#include <semiDB.h>

//#include <iostream>

//using namespace std;

void dbManager::DB_start()
{
	DB_socket();
	DB_connect();

}

void dbManager::DB_socket()
{
	if ((dbSock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket ");
		return;
	}
	memset(&addr, 0x00, sizeof(addr));

	cout << "socket ready" << endl;
}

void dbManager::DB_connect()
{
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("192.168.8.32");// 192.168.8.32  // 127.0.0.1
	addr.sin_port = htons(DBPORT);

	cout << "DB CONNECT TRYING" << endl;

	while (1)
	{
		// DB 서버 접속이 안될 시 1초마다 재접속
		if (connect(dbSock, (struct sockaddr*) &addr, sizeof(addr)) < 0)
		{
			cout << "DB not Connected" << endl;
			//perror("DB connect ");
			//return;
		}
		else
			break;

		usleep(1000000);
	}
	cout << "DB CONNECT FINISHED" << endl;
}



bool dbManager::DB_loginREQ(char* data, int len)
{
	DB_mutex.lock();

	char DBrecv_buf[sizeof(PACKET) + 1] = { 0 };
	//char user_id[50] = { 0 };
	//char user_pw[50] = { 0 };
	PACKET resPack;

	cout << "dbSock : " << dbSock << endl;

	// 05.12 HACK -  연결 도중 SEND ERROR : Bad address  발생
	if (send(dbSock, data, len, 0) == -1)
	{
		//cout << strerror(errno) << endl;
		//perror("send");
		cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
		cout << "Send Error Occured" << endl;
		cout << "Errno : " << errno << endl;
		cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
		return false;
	}

	cout << "DB read Ready" << endl;

	recv_len = read(dbSock, &DBrecv_buf, sizeof(PACKET) + 1);
	cout << "DB read Finished" << endl;

	decodePacket(DBrecv_buf, resPack);

	DB_mutex.unlock();

	if(resPack.body.cmd == EM_CMD_USER_LOGIN_REQ_SUCCESS)
	{
		return true;
	}
	else if(resPack.body.cmd == EM_CMD_USER_LOGIN_REQ_FAIL)
	{
		return false;
	}



	cout << "ERROR DB RECV" << endl;
	return false;
}


























