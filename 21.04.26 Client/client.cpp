#include "headers.h"
#include "packet.h"

#include <random>

#define MAX_DATA 6000
#define MAX_CLIENT 100



class client
{
private:
	int sock;
	struct sockaddr_in addr;
	string msg;
	int recv_len;
	int t_id;

	mutex send_mtx;
	mutex recv_mtx;

	pthread_t send_handle;
	pthread_t recv_handle;

public:

	void client_start(int id)
	{
		t_id = id;
		cl_socket();
		cl_connect();
		cl_mainLoop();
	}

	void cl_socket()
	{ 
		if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			perror("socket ");
			return;
		}
		memset(&addr, 0x00, sizeof(addr));

		cout << "socket ready" << endl;
	}

	void cl_connect()
	{
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		addr.sin_port = htons(PORT);

		if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		{
			perror("connect ");
			return;
		}

		cout << "connect finish" << endl;
	}

	void cl_recv()
	{
		cout << "recv start" << endl;

		while (1)
		{	
			char recv_buf[MAX_DATA];
			memset(&recv_buf, 0, MAX_DATA);
			
			if((recv_len = recv(sock, recv_buf, MAX_DATA, 0)) < 0)
			{
				perror("recv");
				break;
			}

			PACKET *recvPacket = (PACKET *)recv_buf;

			switch (recvPacket->body.cmd)
			{
			case EM_CMD_USER_CHAT_RESULT:
				cout << "RECVED : " << recvPacket->body.data << endl;
				break;

			case EM_CMD_USER_LOG_RESULT:
				cout << "---------------LOG-------------------" << endl;
				//cout << recvPacket->body.data << endl;
				cout << "--------------END LOG----------------" << endl;
				break;

			case EM_CMD_USER_LOG_DELETE_RESULT:
				cout << recvPacket->body.data << endl;
				break;

			default:
				break;
			}

		
			usleep(10000);
		}
	}

	void cl_send()
	{
		cout << "send start" << endl;
		
		while (1)
		{
			char send_buf[MAX_DATA];
			PACKET pack;
			memset(&pack.body.data, 0, sizeof(pack.body.data));
			memset(&send_buf, 0, MAX_DATA);
			getline(cin, msg);

			// packet
			if(msg == "exit")
			{
				break;
			}
			else if(msg == "log")
			{
				pack.body.cmd = EM_CMD_USER_LOG_REQ;
			}
			else if(msg == "delete")
			{
				pack.body.cmd = EM_CMD_USER_LOG_DELETE_REQ;
			}
			else
			{
				pack.body.cmd = EM_CMD_USER_CHAT_REQ;
				strncpy(pack.body.data, msg.c_str(), msg.length());
				pack.phead.datasize = strlen(pack.body.data);
			}


			if(send(sock, (char*) &pack, sizeof(PACKET), 0) == -1)
			{
				perror("send");
				return;	
			}


			usleep(10000);
		}

		cl_exit();
	}

	void dummy_recv()
	{
		cout << "recv start" << endl;

		while (1)
		{
			char recv_buf[MAX_DATA];
			memset(&recv_buf, 0, MAX_DATA);

			if ((recv_len = recv(sock, recv_buf, MAX_DATA, 0)) <= 0)
			{
				perror("recv");
				break;
			}

			PACKET *recvPacket = (PACKET *)recv_buf;

			cout << "Client [" << t_id << "]  recved --> ";

			switch (recvPacket->body.cmd)
			{
			case EM_CMD_USER_CHAT_RESULT:
			//	cout << "RECVED : " << recvPacket->body.data << endl;
				cout << "CHAT_RESULT" << endl;
				break;

			case EM_CMD_USER_LOG_RESULT:
			
				cout << "\n---------------LOG-------------------" << endl;
				cout << recvPacket->body.data;
				cout << "--------------END LOG----------------" << endl;
				//cout << "LOG_RESULT" << endl;
				break;

			case EM_CMD_USER_LOG_DELETE_RESULT:
				//cout << recvPacket->body.data << endl;
				cout << "DELETE_RESULT" << endl;
				break;

			default:
				break;
			}

			usleep(10000);
		}
	}

	void dummy_send()
	{
		// 난수 생성
		random_device rd;
		mt19937 gen(rd());
		uniform_int_distribution<int> dis(0, 99);

		cout << "dummy send start" << endl;

		while (1)
		{
			char send_buf[MAX_DATA];
			PACKET pack;
			memset(&pack.body.data, 0, sizeof(pack.body.data));
			memset(&send_buf, 0, MAX_DATA);
			int rdNum = dis(gen) % 10;
			int delay = dis(gen) % 4000000 + 4000000;	// 1,000,000 = 1초

			vector<string> dummyWord = 
			{
				"one", "two", "three", "four", "five",
				"six", "seven" ,"eight" ,"log", "delete"
			};
			//getline(cin, msg);

			msg = dummyWord[rdNum];

			// packet
			if (msg == "exit")
			{
				break;
			}
			else if (msg == "log")
			{
				pack.body.cmd = EM_CMD_USER_LOG_REQ;
			}
			else if (msg == "delete")
			{
				pack.body.cmd = EM_CMD_USER_LOG_DELETE_REQ;
			}
			else
			{
				pack.body.cmd = EM_CMD_USER_CHAT_REQ;
				strncpy(pack.body.data, msg.c_str(), msg.length());
				pack.phead.datasize = strlen(pack.body.data);
			}

			if (send(sock, (char *)&pack, sizeof(PACKET), 0) == -1)
			{
				perror("send");
				return;
			}

			usleep(delay);

		}

		cl_exit();
	}

	void cl_mainLoop()
	{
		//thread sendThread(&client::cl_recv, this);
		// thread recvThread(&client::cl_send, this);
		thread sendThread(&client::dummy_recv, this);
		thread recvThread(&client::dummy_send, this);

		send_handle = sendThread.native_handle();
		recv_handle = recvThread.native_handle();

		sendThread.join();
		recvThread.join();
	}

	void cl_exit()
	{
		close(sock);
		pthread_cancel(send_handle);
		pthread_cancel(recv_handle);
		cout << "----------------- CLOSED --------------------" << endl;
		cout << endl;
	}
};

int main(void)
{
	//client mg_client;

	//mg_client.client_start();

	client cl[MAX_CLIENT];
	thread t[MAX_CLIENT];
	for (int i = 0; i < MAX_CLIENT; i++)
	{
		//thread *t = new thread(&client::client_start, &cl[i]);
		t[i] = std::thread(&client::client_start, &cl[i], i + 1);
		usleep(200000);
	}


	for(int i = 0; i < MAX_CLIENT; i++)
	{
		t[i].join();
	}
}

//  g++ -pthread client.cpp






