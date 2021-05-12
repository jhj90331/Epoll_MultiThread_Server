
#ifndef PACKET_DEFINE_H_
#define PACKET_DEFINE_H_

#define MAX_DATA_SIZE 1024
#define MAX_BODY_SIZE (MAX_DATA_SIZE + 1 + sizeof(packetType) + sizeof(unsigned short))
#define MAX_PRINT_DATA_SIZE 1024
#define NO_BODY_PACKET_SIZE 18

#include <string>

enum packetType
{
	EM_CMD_USER_CHAT_REQ = 100,
	EM_CMD_USER_CHAT_RESULT,
	EM_CMD_USER_LOG_REQ ,
	EM_CMD_USER_LOG_RESULT,
	EM_CMD_USER_LOG_DELETE_REQ,
	EM_CMD_USER_LOG_DELETE_RESULT,
	EM_CMD_USER_LOGIN_REQ,
	EM_CMD_USER_LOGIN_REQ_SUCCESS,
	EM_CMD_USER_LOGIN_REQ_FAIL
};

#pragma pack(push, 1)	//1byte 정렬

typedef struct PACKET_HEADER
{
	char head[5] = {"AA11"};
	unsigned short bodysize;
}HEADER;

typedef struct PACKET_BODY
{
	enum packetType cmd;
	unsigned short datasize;
	char data[MAX_DATA_SIZE+1] = {0};
}BODY;

typedef struct PACKET_TAIL
{
	char tail[5] = {"11AA"};
}TAIL;

typedef struct MILESTONE_PACKET
{
	HEADER phead;
	BODY body;
	TAIL ptail;
}PACKET;

void makePacket(packetType pkType ,char* bodyData, PACKET &pk);

void encodePacket(char* packetArr, PACKET pk);

void decodePacket(char* packetArr, PACKET &pk);


#pragma pack(pop)	//정렬 설정을 이전 상태(기본값)로 되돌림


#endif /* PACKET_DEFINE_H_ */





























