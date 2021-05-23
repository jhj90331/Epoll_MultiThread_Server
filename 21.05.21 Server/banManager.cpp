/*
 * banManager.cpp
 *
 *  Created on: 2021. 5. 21.
 *      Author: hsr
 */

#include <banManager.h>

void banManager::start(int delay, int maxNum)
{
	m_delay = delay;
	m_MAXconnectNum = maxNum;
	m_checkingNow = false;
}


void banManager::add_IP(string ban_IP)
{
	if(m_checkingNow == false)
	{

		m_banMutex.lock();

		auto itr = m_IPcheckMap.find(ban_IP);
		if (itr != m_IPcheckMap.end())
		{
			itr->second++;
		}
		else
		{
			m_IPcheckMap.insert(make_pair(ban_IP, 1));
		}

		m_banMutex.unlock();
	}
}

void banManager::clearMap()
{
	m_banMutex.lock();
	m_IPcheckMap.clear();
	m_banMutex.unlock();
}

// 해당 IP가 밴 목록에 있는지 확인
bool banManager::is_Banned(string ban_IP)
{
	if(m_bannedIP.find(ban_IP) != m_bannedIP.end())
	{
		return true;
	}
	else
		return false;
}

// delay 시간동안 connect 요청 한 횟수가 MAX connect 보다 많을경우 밴
void banManager::banThreadWork()
{
	while(1)
	{
		m_checkingNow = false;
		usleep(m_delay);
		m_checkingNow = true;

		for(auto &client : m_IPcheckMap)
		{
			if( (client.second >= m_MAXconnectNum) && (m_bannedIP.find(client.first) == m_bannedIP.end()) )
			{
				m_banMutex.lock();
				m_bannedIP.insert(client.first);
				cout << "ADD BANNED IP : " << client.first << ", TRY Num : " << client.second << endl;
				m_banMutex.unlock();
			}
			else
			{
				cout << "Check IP : " << client.first << ", Check Num : " << client.second << endl;
			}
		}
		clearMap();
	}
}




