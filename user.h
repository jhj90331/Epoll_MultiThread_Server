/*
 * user.h
 *
 *  Created on: 2021. 4. 2.
 *      Author: hsr
 */

#ifndef USER_H_
#define USER_H_

#include <headers.h>

class user
{
private:
	int m_client_fd;		// 유저의 파일 디스크럽터
	vector<string> m_log; // 유저가 보내는 메시지를 저장하는 벡터
	//mutex m_logMutex;

public:
	user(int fd);
	~user();
	void set_fd(int fd); // fd 설정
	int get_fd();			// fd 받아옴
	bool is_emptyLog();	// m_log가 비어있는지 확인
	void clear_Log();		// m_log 비우기
	string get_Log();		// log 반환
	void add_Log(string str);	// 로그에 추가

};


#endif /* USER_H_ */
