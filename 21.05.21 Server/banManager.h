/*
 * banManager.h
 *
 *  Created on: 2021. 5. 21.
 *      Author: hsr
 */

#ifndef BANMANAGER_H_
#define BANMANAGER_H_

#include <headers.h>
#include <packet.h>

class banManager
{
private:
	set<string> m_bannedIP;
	map<string, unsigned int> m_IPcheckMap;
	mutex m_banMutex;
	unsigned int m_delay;
	unsigned int m_MAXconnectNum;
	bool m_checkingNow;

public:
	void start(int delay, int maxNum);
	void add_IP(string ban_IP);
	void clearMap();
	bool is_Banned(string ban_IP);
	void banThreadWork();
};


#endif /* BANMANAGER_H_ */
