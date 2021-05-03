/*
 * circular_Buffer.h
 *
 *  Created on: 2021. 4. 20.
 *      Author: hsr
 */

#ifndef CIRCULAR_BUFFER_H_
#define CIRCULAR_BUFFER_H_

#include <headers.h>
#include <packet.h>

#define CIRCULAR_MAX_SIZE 6500

class circular_Buffer
{
private:
	char m_buffer[CIRCULAR_MAX_SIZE];
	int m_writeOffset;
	int m_readOffset;
	int m_currentSize;
	int m_restSize;

public:
	circular_Buffer();
	~circular_Buffer();

	bool enqueue(char* data, int len);
	bool dequeue(char* result, int len);
	bool is_empty();
	int get_curSize();
	int get_restSize();

};





#endif /* CIRCULAR_BUFFER_H_ */
