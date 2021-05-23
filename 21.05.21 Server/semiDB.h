/*
 * semiDB.h
 *
 *  Created on: 2021. 5. 10.
 *      Author: hsr
 */

#ifndef SEMIDB_H_
#define SEMIDB_H_


#include <headers.h>
#include <packet.h>

class dbManager
{
private:
	int dbSock;
	struct sockaddr_in addr;
	string msg;
	int recv_len;
	mutex DB_mutex;

public:
	void DB_start();
	void DB_socket();
	void DB_connect();
	bool DB_loginREQ(char* data, int len);


};


#endif /* SEMIDB_H_ */
