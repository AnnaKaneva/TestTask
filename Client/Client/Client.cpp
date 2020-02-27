// Client.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "UDP_TCP.h"
#include <iostream>

using namespace std;

int main()
{
	int prot = 0;
	//TODO: limited times of enter
	for (int i = 0; i < 3; i++)
	{
		cout << "Choose transport protocol.\nEnter 1 for TCP and 2 for UDP." << endl;
		cin >> prot;

		if ((prot == 1) || (prot == 2))
		{
			break;
		}

		if (i == 2)
		{
			cout << "Last try was incorrect.\nClient closing." << endl;
			return 1;
		}

		cout << "Incorrect number were entered.\nTry again." << endl;
	}
	
	WSADATA wsaData;
	int err = WSAStartup(MAKEWORD(1, 1), &wsaData);

	if (err == SOCKET_ERROR)
	{
		cout << "WSAStartup() failed: " << GetLastError() << endl;
		return 1;
	}

	CTransport  * clnt;

	if (prot == 1) {
		clnt = new(nothrow) CTCP();
	} 
	else {
		clnt = new(nothrow) CUDP();
	}

	if (clnt->Connection("192.168.1.70") != S_OK)
	{
		return 1;
	}

	cin.ignore();

	for (;;)
	{
		cout << "Enter the text:" << endl;
		
		char * text = (char*)HALLOC(MAXPACKETSIZE);

		if (!text)
		{
			cout << "Couldn't allocate the memory" << endl;
			return 1;
		}	

		cin.getline(text, MAXPACKETSIZE);

		if (clnt->Send((BYTE *)text, strlen(text)) != S_OK)
		{
			if (text)
			{
				HFREE(text);
				text = NULL;
			}
			
			return 1;
		}

		if (clnt->Receive() != S_OK)
		{
			if (text)
			{
				HFREE(text);
				text = NULL;
			}
			
			return 1;
		}

		if (text)
		{
			HFREE(text);
			text = NULL;
		}
		
	}

	WSACleanup();
    return 0;
}

