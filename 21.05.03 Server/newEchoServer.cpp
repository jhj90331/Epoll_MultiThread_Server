//============================================================================
// Name        : newEchoServer.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <headers.h>
#include <mgServer.h>


int main()
{

	mgServer server;

	server.mg_server_start();

	return 0;
}
