/*
 * userManager.cpp
 *
 *  Created on: 2021. 4. 5.
 *      Author: hsr
 */

#include <userManager.h>

userManager::userManager()
{
}

user* userManager::find_user(int client_fd)
{
	m_userMap_Mutex.lock();
	user* res = m_userMap.find(client_fd)->second;
	m_userMap_Mutex.unlock();
	return res;
}

void userManager::add_user(int client_fd, user* conUser)
{
	cout << "ADD USER IN" << endl;
	cout << "userMap SIZE : " << m_userMap.size() << endl;

	m_userMap_Mutex.lock();
	m_userMap.insert(make_pair(client_fd, conUser));
	m_userMap_Mutex.unlock();
}

void userManager::del_user(int client_fd)
{
	m_userMap_Mutex.lock();
	m_userMap.erase(m_userMap.find(client_fd));
	m_userMap_Mutex.unlock();
}

// 유저가 있는지 없는지 검사
bool userManager::is_user_in(int client_fd)
{
	if(m_userMap.find(client_fd) != m_userMap.end())
		return true;
	else
		return false;
}

void userManager::is_alive_thread()
{
	while(1)
	{
		for (auto itr = m_userMap.begin(); itr != m_userMap.end(); )
		{
			m_userMap_Mutex.lock();

			if (itr->second->is_alive() == false)
			{
				m_userMap.erase(itr);
				cout <<"USERMAP ERASED" << endl;
				cout << "Client FD :" << itr->second->get_fd() << endl;
				cout << "m_userMap SIZE : " <<  m_userMap.size() << endl;

				close(itr->second->get_fd());
				delete itr->second;
			}
			else
			{
				itr++;
			}

			m_userMap_Mutex.unlock();
		}

		//m_userMap_Mutex.unlock();

		usleep(1000);
	}
}















