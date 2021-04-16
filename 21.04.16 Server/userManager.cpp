/*
 * userManager.cpp
 *
 *  Created on: 2021. 4. 5.
 *      Author: hsr
 */

#include <userManager.h>

user &userManager::find_user(int client_fd)
{
	user &res = m_userMap.find(client_fd)->second;

	return res;
}

void userManager::add_user(int client_fd)
{
	user tmp(client_fd);

	m_userMap.insert(make_pair(client_fd, tmp));
}

void userManager::del_user(int client_fd)
{
	m_userMap.erase(m_userMap.find(client_fd));
}


















