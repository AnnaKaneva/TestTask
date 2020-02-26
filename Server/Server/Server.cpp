// Server.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "UDP_TCP.h"


DWORD UDPloop(CUDP * transp)
{
	for (;;)
	{
		int err = 0;
		if ((err = transp->Receive()) == S_OK)
		{
			if (transp->Send() != S_OK)
			{
				return 1;
			}
		}
		else if (err != WSAETIMEDOUT)
		{
			return 1;
		}
	}
}


int main()
{
	WSADATA wsaData;
	int err = WSAStartup(MAKEWORD(1, 1), &wsaData);

	if (err == SOCKET_ERROR)
	{
		printf("WSAStartup() failed: %ld\n", GetLastError());
		return 1;
	}

	CTransport * UDP = new CUDP();
	UDP->Bind();
	UDPloop((CUDP*)UDP);

	/*CTransport * UDP = new CUDP();
	UDP->Bind();
	while (1)
	{
		if (UDP->Receive() == S_OK)
		{
			UDP->Send();
		}
	}*/
	/*CTransport * TCP = new CTCP();
	TCP->Listen();
	TCP->Temperary();*/

	WSACleanup();
    return 0;
}

