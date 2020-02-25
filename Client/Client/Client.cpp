// Client.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "UDP_TCP.h"

int main()
{
	int prot = 0;
	while (1)
	{
		printf("Choose transport protocol.\nEnter 1 for TCP and 2 for UDP.\n");
		scanf_s("%d", &prot);

		if ((prot == 1) || (prot == 2))
		{
			break;
		}

		printf("Incorrect number were entered.\nTry again.\n");
	}
	
	WSADATA wsaData;
	WSAStartup(MAKEWORD(1, 1), &wsaData);

	CTransport  * clnt;

	if (prot == 1) {
		clnt = new CTCP();
	} 
	else {
		clnt = new CUDP();
	}
	
	clnt->Connection("192.168.1.70");

	clnt->Send((BYTE *)"Well, hello there", 17);

	clnt->Receive();

	WSACleanup();
    return 0;
}

