/*
 * userManager.h
 *
 *  Created on: 2021. 4. 2.
 *      Author: hsr
 */

#ifndef USERMANAGER_H_
#define USERMANAGER_H_

#include <headers.h>
#include <user.h>
#include <packet.h>


class userManager
{
private:
	unordered_map<int, user> m_userMap;

public:
	user &find_user(int client_fd);
	void add_user(int client_fd);
	void del_user(int client_fd);

	bool is_user_in(int client_fd);
};



#endif /* USERMANAGER_H_ */
