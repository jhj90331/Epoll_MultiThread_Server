/*
 * user.cpp
 *
 *  Created on: 2021. 4. 2.
 *      Author: hsr
 */

#include <user.h>

user::user(int fd)
{
	m_client_fd = fd;
	//m_log.clear();
}

user::~user()
{

}

void user::set_fd(int fd)
{
	m_client_fd = fd;
}

int user::get_fd()
{
	return m_client_fd;
}

bool user::is_emptyLog()
{
	//m_logMutex.lock();

	if(m_log.empty() == true)
		return true;
	else
		return false;

	//m_logMutex.unlock();
}

void user::clear_Log()
{
	//m_logMutex.lock();

	if(m_log.empty() != true)
		m_log.clear();

	//m_logMutex.unlock();
}

string user::get_Log()
{
	//m_logMutex.lock();

	string res;
	int size = m_log.size();

	for(int i = 0; i < size; i++)
	{
		res += m_log[i];
		res.push_back('\n');
	}

	//m_logMutex.unlock();
	return res;
}

void user::add_Log(string str)
{
	//m_logMutex.lock();

	m_log.push_back(str);
	if(m_log.size() >= 20)
	{
		m_log.erase(m_log.begin());
	}

	//m_logMutex.unlock();
}












