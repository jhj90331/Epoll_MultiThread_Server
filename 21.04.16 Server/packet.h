/*
 * packet.h
 *
 *  Created on: 2021. 4. 2.
 *      Author: hsr
 */

#ifndef PACKET_DEFINE_H_
#define PACKET_DEFINE_H_

#define MAX_DATA_SIZE 1024
#define MAX_PRINT_DATA_SIZE 1024

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
	unsigned int datasize;
}HEADER;

typedef struct PACKET_BODY
{
	enum packetType cmd;
	char data[MAX_DATA_SIZE+1];
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

#pragma pack(pop)	//정렬 설정을 이전 상태(기본값)로 되돌림





#endif /* PACKET_DEFINE_H_ */




