#include <packet.h>
#include <headers.h>


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























