/*
 * circular_Buffer.cpp
 *
 *  Created on: 2021. 4. 20.
 *      Author: hsr
 */

#include <circular_Buffer.h>

circular_Buffer::circular_Buffer()
{
	memset(m_buffer, 0, CIRCULAR_MAX_SIZE);
	m_writeOffset = 0;
	m_readOffset = 0;
	m_currentSize = 0;
	m_restSize = CIRCULAR_MAX_SIZE;
}


circular_Buffer::~circular_Buffer()
{
}

bool circular_Buffer::is_empty()
{
	if(m_currentSize == 0)
		return true;
	else
		return false;
}

int circular_Buffer::get_curSize()
{
	return m_currentSize;
}

int circular_Buffer::get_restSize()
{
	return m_restSize;
}

bool circular_Buffer::enqueue(char* data, int len)
{
	if(m_restSize < len)
		return false;

	if(m_writeOffset + len < CIRCULAR_MAX_SIZE)
	{
		memcpy(m_buffer + m_writeOffset, data, len);
		m_writeOffset += len;
	}
	else
	{
		int rest = (m_writeOffset + len) - CIRCULAR_MAX_SIZE;
		memcpy(m_buffer + m_writeOffset, data, len - rest);
		memcpy(m_buffer, data + (len - rest), rest);
		m_writeOffset = rest;
	}


	m_currentSize += len;
	m_restSize -= len;

	//cout << "CirCular Enque, Write Offset : " << m_writeOffset << endl;

	return true;
}


bool circular_Buffer::dequeue(char* result, int len)
{
	if(m_currentSize < len)
		return false;

	 if(m_readOffset + len < CIRCULAR_MAX_SIZE)
	 {
		 memcpy(result, m_buffer + m_readOffset, len);
		 m_readOffset += len;
	 }
	 else
	 {
		 int rest = (m_readOffset + len) - CIRCULAR_MAX_SIZE;
		 memcpy(result, m_buffer + m_readOffset, len - rest);
		 memcpy(result + (len - rest), m_buffer, rest);
		 m_readOffset = rest;
	 }

	 m_currentSize -= len;
	 m_restSize += len;

	 //cout << "CirCular Deque, Read Offset : " << m_readOffset << endl;

	 return true;
}











