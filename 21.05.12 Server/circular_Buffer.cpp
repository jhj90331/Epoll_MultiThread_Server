/*
 * circular_Buffer.cpp
 *
 *  Created on: 2021. 4. 20.
 *      Author: hsr
 */

#include <circular_Buffer.h>

circular_Buffer::circular_Buffer()
{
	cBuf_flush();
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

bool circular_Buffer::pop(int len)
{
	if(m_currentSize < len)
			return false;

		 if(m_readOffset + len < CIRCULAR_MAX_SIZE)
		 {
			 m_readOffset += len;
		 }
		 else
		 {
			 int rest = (m_readOffset + len) - CIRCULAR_MAX_SIZE;
			 m_readOffset = rest;
		 }

		 m_currentSize -= len;
		 m_restSize += len;

		 return true;
}


void circular_Buffer::cBuf_flush()
{
	memset(m_buffer, 0, CIRCULAR_MAX_SIZE);
	m_writeOffset = 0;
	m_readOffset = 0;
	m_currentSize = 0;
	m_restSize = CIRCULAR_MAX_SIZE;
}

int circular_Buffer::cBuf_strstr(char* src, char* findStr, int curSize)
{
	int find_Len = strlen(findStr);
	int result = -1;

	for (int i = 0; find_Len + i < curSize; i++)
	{
		result = -1;

		for (int j = 0; j < find_Len; j++)
		{
			if (*(src + i + j) == *(findStr + j))
			{
				if (j == find_Len - 1)
				{
					result = i;
					return result;
				}
				else
					continue;
			}
			else
				break;
		}
	}


	return result;
}

int circular_Buffer::cBuf_find(char* findStr)
{
	// offset 백업
	int tmp_ReadOffset = m_readOffset;
	int tmp_CurrentSize = m_currentSize;
	int tmp_RestSize = m_restSize;

	char tmpStr[CIRCULAR_MAX_SIZE + 1] = {0};
	dequeue(tmpStr, m_currentSize);

	//char* find_ptr = strstr(tmpStr, findStr);

	int find_ptr = cBuf_strstr(tmpStr, findStr, tmp_CurrentSize);


	if(find_ptr == -1)
	{
		m_readOffset = tmp_ReadOffset;
		m_currentSize = tmp_CurrentSize;
		m_restSize = tmp_RestSize;
		return -1;
	}
	else
	{
		m_readOffset = tmp_ReadOffset;
		m_currentSize = tmp_CurrentSize;
		m_restSize = tmp_RestSize;
		return find_ptr;
	}
}

















