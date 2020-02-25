// Server.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "UDP_TCP.h"


int main()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(1, 1), &wsaData);

	/*CTransport * UDP = new CUDP();
	UDP->Listen();
	while (1)
	{
		if (UDP->Receive() == S_OK)
		{
			UDP->Send();
		}
	}*/
	CTransport * TCP = new CTCP();
	TCP->Listen();
	TCP->Temperary();

	WSACleanup();
    return 0;
}

