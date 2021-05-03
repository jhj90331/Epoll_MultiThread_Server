
#ifndef PACKET_DEFINE_H_
#define PACKET_DEFINE_H_

#define MAX_DATA_SIZE 1024
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
	EM_CMD_USER_LOG_DELETE_RESULT
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

void makePacket(packetType pkType ,char* bodyData, PACKET &pk)
{
	// BODY
	pk.body.cmd = pkType;
	pk.body.datasize = strlen(bodyData) + 1;  // +1 은 문자열 마지막의 \0
	memcpy(pk.body.data, bodyData, strlen(bodyData) + 1);

	// HEAD
	pk.phead.bodysize = sizeof(packetType) + sizeof(unsigned short) + pk.body.datasize; // 4 + 2 + data
}



void encodePacket(char* packetArr, PACKET pk)
{
	int offset = 0;

	// HEADER 복사
	memcpy(packetArr, &pk, sizeof(HEADER)); 
	offset += sizeof(HEADER);

	// BODY 복사
	memcpy(packetArr + offset, &pk.body, pk.phead.bodysize);
	offset += pk.phead.bodysize;

	// TAIL 복사
	memcpy(packetArr + offset, &pk.ptail, sizeof(TAIL));

}

void decodePacket(char* packetArr, PACKET &pk)
{
	int offset = 0;

	// HEADER 복사
	memcpy(&pk.phead, packetArr, sizeof(HEADER));
	offset += sizeof(HEADER);

	// BODY 복사
	memcpy(&pk.body, packetArr + offset, pk.phead.bodysize);
	offset += pk.phead.bodysize;

	// TAIL
	// 11AA로 동일
}


#pragma pack(pop)	//정렬 설정을 이전 상태(기본값)로 되돌림


#endif /* PACKET_DEFINE_H_ */





























