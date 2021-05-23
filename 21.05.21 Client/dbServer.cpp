
#include "headers.h"
#include "packet.h"

int main(void)
{
    int sock, client_sock;
    struct sockaddr_in addr, client_addr;
    //char recv_buf[sizeof(PACKET) + 1];
    int len, addr_len, recv_len;
    mutex DB_mutex;

    map<string, string> loginTable =
        {
            {"test0", "1q2w3e4r"}, {"test1", "password1"}, {"test2", "password2"}, {"test3", "password3"},
            {"test4", "password4"}, {"test5", "password5"}, {"test6", "password6"}, {"test7", "password7"},
            {"test8", "password8"}, {"test9", "password9"}

        };

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket ");
        return 1;
    }
    memset(&addr, 0x00, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(DBPORT);
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind ");
        return 1;
    }
    if (listen(sock, 5) < 0)
    {
        perror("listen ");
        return 1;
    }

    ///////////////////////////////////////////////////////////////////////////

    addr_len = sizeof(client_addr);
    cout << "Waiting For Client" << endl;

    client_sock = accept(sock, (struct sockaddr *)&client_addr, (socklen_t *)&addr_len);

    cout << "DB Client Accepted" << endl;

    while (1)
    {
        char recv_buf[sizeof(PACKET) + 1] = {0};
        cout << "Recv Ready" << endl;

        if ((recv_len = recv(client_sock, recv_buf, sizeof(PACKET), 0)) < 0)
        {
            perror("recv ");
            return 1;
            break;
        }

        if (strncmp(recv_buf, "AA11", 4) != 0)
        {
            cout << "HEADER IS NOT AA11" << endl;
           // break;
        }

        cout << "RECVED SIZE : " << recv_len << endl;

        PACKET recvPack;
        decodePacket(recv_buf, recvPack);
        char id[50] = {0};
        char pw[50] = {0};

        PACKET resPack;
        string resultStr = "user Login Fail";
        packetType result_Type = EM_CMD_USER_LOGIN_REQ_FAIL;

        //////////////////////////////////////////////
        // 21.05.13 TODO : GOTO 로직 수정 하기//
        /////////////////////////////////////////////
        if (recvPack.body.cmd == EM_CMD_USER_LOGIN_REQ)
        {
            if (strchr(recvPack.body.data, '\n') == NULL)
            {
                // ID와 PW 경계가 없을 때
                cout << "No Bound Packet" << endl;
                goto GOTO_skip;
            }

            int len = strlen(recvPack.body.data);
            int bound = strchr(recvPack.body.data, '\n') - recvPack.body.data;

            // 경계가 2개 이상일 때
            if (strchr(recvPack.body.data + bound + 1, '\n') != NULL)
            {
                cout << "Multiple Bound Packet" << endl;
                goto GOTO_skip;
            }
            // 경계가 양 끝에 있을 때
            if (recvPack.body.data[0] == '\n' || recvPack.body.data[recvPack.phead.bodysize] == '\n')
            {
                cout << "Edge Bound Packet" << endl;
                goto GOTO_skip;
            }
            strncpy(id, recvPack.body.data, bound);
            strncpy(pw, recvPack.body.data + bound + 1, len - bound);

            cout << "AFTER ID PW DECODE" << endl;

            string user_id(id);
            string user_pw(pw);

            // ID, PW 최대 사이즈 검사
            if (user_id.length() >= MAX_ID_SIZE || user_pw.length() >= MAX_PW_SIZE)
            {
                cout << "User ID / PW Size Over" << endl;
                goto GOTO_skip;
            }

            cout << "LOGIN REQ - ID : " << user_id << ", PW : " << user_pw << endl;

            // 로그인 테이블에서 find
            if (loginTable.find(user_id) != loginTable.end())
            {
                if (loginTable.find(user_id)->second == user_pw)
                {
                    result_Type = EM_CMD_USER_LOGIN_REQ_SUCCESS;
                    resultStr = "user Login Success";
                    cout << resultStr << endl;
                }
                // else 
                //{
                //    result_Type = EM_CMD_USER_LOGIN_REQ_FAIL;
                //    resultStr = "user Login Fail";
                //   cout << resultStr << endl;
                //}
            }
            //else
            //{
            //    result_Type = EM_CMD_USER_LOGIN_REQ_FAIL;
            //    resultStr = "user Login Fali";
            //    cout << resultStr << endl;
            //}
        }

    GOTO_skip:
        
        if(result_Type== EM_CMD_USER_LOGIN_REQ_FAIL)
        {
            cout << "user Login Fail" << endl;
        }

        char dataTmp[sizeof(PACKET)] = {0};
        char send_buf[sizeof(PACKET) + 1] = {0};
        strncpy(dataTmp, resultStr.c_str(), resultStr.length());
        makePacket(result_Type, dataTmp, resPack);
        encodePacket(send_buf, resPack);


        cout << "Send Ready" << endl;
        
        if (send(client_sock, send_buf, sizeof(PACKET) + 1, 0) == -1)
        {
            cout << "Login Send Error" << endl;
            perror("Login Send");
        }

        cout << "Process Complete" << endl;

        usleep(10);
    }

    close(sock);
    return 0;
}
