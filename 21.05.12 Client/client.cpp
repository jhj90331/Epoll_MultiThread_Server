#include "headers.h"
#include "packet.h"

#include <random>

#define MAX_DATA 6000
#define MAX_BODY_DATA_SIZE 1500
#define MAX_CLIENT 100


int restThreadNum = MAX_CLIENT;
mutex restMutex;

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

	bool connected;

public:

	void client_start(int id)
	{
		t_id = id;
		cl_socket();
		cl_connect();
		connected = true;
		if(cl_login() == true)
		{
			cout << "cl_Login Success" << endl;
			cl_mainLoop();
		}
		else
		{
			cout << "cl_Login Failed" << endl;
			cl_exit();
		}
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
		addr.sin_addr.s_addr = inet_addr("192.168.8.32");	// 192.168.8.32  // 127.0.0.1
		addr.sin_port = htons(PORT);

		if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		{
			cl_exit();
			perror("connect ");
			return;
		}

		cout << "connect finish" << endl;
	}

	bool cl_login()
	{
		cout << "Login Start" << endl;

		char send_buf[MAX_DATA] = {0};
		PACKET pack;

		vector<string> id_List = 
		{
			"test1", "test2", "test3", "test4", "test5",
			"test6", "test7", "test8", "test9", "test0"
		};

		vector<string> pw_List = 
		{
			"password1", "password2", "password3", "password4", "password5",
			"password6", "password7", "password8", "password9", "password0"
		};

		//string loginMsg = "test1\npassword1";
		string loginMsg;
		loginMsg += id_List[t_id % 10];
		loginMsg.push_back('\n');
		loginMsg += pw_List[t_id % 10];


		char dataTmp[MAX_BODY_DATA_SIZE] = {0};
		strncpy(dataTmp, loginMsg.c_str(), loginMsg.length());
		makePacket(EM_CMD_USER_LOGIN_REQ, dataTmp, pack);
		encodePacket(send_buf, pack);

		cout << "Send Start" << endl;
		if (send(sock, send_buf, pack.body.datasize + NO_BODY_PACKET_SIZE, 0) == -1)
		{
			perror("Login Send");
		}
		cout << "Send Finish" << endl;


		char recv_buf[MAX_DATA] = {0};

		cout << "recv Ready" << endl;
		if ((recv_len = recv(sock, recv_buf, MAX_DATA, 0)) <= 0)
		{
			perror("Login Recv");
		}
		cout << "recv Finish" << endl;

		PACKET recvPack;
		decodePacket(recv_buf, recvPack);

		if (recvPack.body.cmd == EM_CMD_USER_LOGIN_REQ_SUCCESS)
		{
			cout << "Recv Packet CMD ==== Login Success" << endl;
			return true;
		}
		else if (recvPack.body.cmd == EM_CMD_USER_LOGIN_REQ_FAIL)
		{
			cout << "Recv Packet CMD ==== Login Fail" << endl;
			return false;
		}

		return false;
	} // cl_login

	void dummy_recv()
	{
		cout << "recv start" << endl;

		while (1)
		{
			char recv_buf[MAX_DATA] = {0};

			if ((recv_len = recv(sock, recv_buf, MAX_DATA, 0)) <= 0)
			{
				cout << "Recv Error" << endl;
				connected = false;
				break;
			}

			//PACKET *recvPacket = (PACKET *)recv_buf;
			PACKET recvPacket;
			decodePacket(recv_buf, recvPacket);

			cout << "Client [" << t_id << "]  recved --> ";

			switch (recvPacket.body.cmd)
			{
			case EM_CMD_USER_CHAT_RESULT:
			//	cout << "RECVED : " << recvPacket->body.data << endl;
				cout << "CHAT_RESULT" << endl;
				break;

			case EM_CMD_USER_LOG_RESULT:
			
				cout << "\n---------------LOG-------------------" << endl;
				cout << recvPacket.body.data;
				cout << "--------------END LOG----------------" << endl;
				//cout << "LOG_RESULT" << endl;
				break;

			case EM_CMD_USER_LOG_DELETE_RESULT:
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
		uniform_int_distribution<int> dis(0, 4000000);

		cout << "dummy send start" << endl;

		while (1)
		{
			char send_buf[MAX_DATA] = {0};
			PACKET pack;

			int rdNum = dis(gen) % 10;
			int delay = (dis(gen) % 4000000) + 4000000;	// 1,000,000 = 1초

			cout << "DELAY : " << delay << endl;

			packetType pkType;

			vector<string> dummyWord = 
			{
				"one111111111111111111111111", "two2222222222222222222222222222", 
				"three333", "four444444", "five55555555555555555555",
				"six66", "seven777777777777777777777" ,"eight888888888888888888888888" ,"log", "delete"
			};

			msg = dummyWord[rdNum];

			// packet
			if (msg == "exit")
			{
				connected = false;
				break;
			}
			else if (msg == "log")
			{
				pkType = EM_CMD_USER_LOG_REQ;
			}
			else if (msg == "delete")
			{
				pkType = EM_CMD_USER_LOG_DELETE_REQ;
			}
			else
			{
				pkType = EM_CMD_USER_CHAT_REQ;
			}

			char dataTmp[MAX_BODY_DATA_SIZE] = {0};
			strncpy(dataTmp, msg.c_str(), msg.length());
			makePacket(pkType, dataTmp, pack);
			

			//////////////////////////////////////////////////////
			if(rdNum == 3)
				BUG_encodePacket1(send_buf, pack);
			if(rdNum == 6)
				BUG_encodePacket2(send_buf, pack);
			else
				encodePacket(send_buf, pack);

			///////////////////////////////////////////////////////

			if (connected == false)
				break;
			if (send(sock, send_buf, pack.body.datasize + NO_BODY_PACKET_SIZE, 0) == -1)
			{
				perror("send");
				break;
			}

			usleep(delay);
		}

		//close(sock);
		//cl_exit();
	}

	void cl_mainLoop()
	{
		//thread sendThread(&client::cl_recv, this);
		// thread recvThread(&client::cl_send, this);
		cout << "Main Loop Start" << endl;

		thread sendThread(&client::dummy_recv, this);
		thread recvThread(&client::dummy_send, this);

		send_handle = sendThread.native_handle();
		recv_handle = recvThread.native_handle();

		sendThread.join();
		recvThread.join();

		cout << "----------------- CLOSED --------------------" << endl;

		cl_exit();
		cout << "REST CLIENT : " << restThreadNum << endl;
		
		cout << "==== rest MUTEX UNLOCKED ====" << endl;
	}

	void cl_exit()
	{
		/*
		cout << "CLIENT EXIT IN" << endl;
		cout << "SOCK : " << sock << endl;
		pthread_cancel(send_handle);
		cout << "SEND THREAD EXITED" << endl;
		pthread_cancel(recv_handle);
		cout << "RECV THREAD EXITED" << endl;
		close(sock);
		
		cout << "----------------- CLOSED --------------------" << endl;
		cout << "server Ended"<< endl;
		*/
		restMutex.lock();

		close(sock);
		restThreadNum--;

		restMutex.unlock();

		cout << "cl_EXITED" << endl;
	}
};

int main(void)
{
	//client mg_client;

	//mg_client.client_start();

	client cl[MAX_CLIENT];
	//thread t[MAX_CLIENT];
	for (int i = 0; i < MAX_CLIENT; i++)
	{
		thread *t = new thread(&client::client_start, &cl[i], i+1);
		//t[i] = std::thread(&client::client_start, &cl[i], i + 1);
		t->detach();
		usleep(200000);
	}

	

	//for(int i = 0; i < MAX_CLIENT; i++)
	//{
	//	t[i].join();
	//}

	while(1)
	{
		if(restThreadNum == 0)
		{
			cout << "REST THREAD ZERO" << endl;
			usleep(5000000);
			return 0;
		}

		usleep(1000000);
	}
}

//  g++ -pthread client.cpp






