// Server.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "UDP_TCP.h"
#include <iostream>
using namespace std;


DWORD WINAPI UDPloop(LPVOID transp)
{
	for (;;)
	{
		int err = 0;
		if ((err = ((CUDP*)transp)->Receive()) == S_OK)
		{
			if (((CUDP*)transp)->Send() != S_OK)
			{
				::SetEvent(((CUDP*)transp)->m_hCloseEvent);
				cout << "Setting closing event in UDP thread" << endl;
				return 1;
			}
		}
		else if (err != WSAETIMEDOUT)
		{
			::SetEvent(((CUDP*)transp)->m_hCloseEvent);
			cout << "Setting closing event in UDP thread" << endl;
			return 1;
		}
	}
}


DWORD WINAPI TCPloop(LPVOID transp)
{
	return ((CTCPMain*)transp)->Accept();
}


int main()
{
	WSADATA wsaData;
	int err = WSAStartup(MAKEWORD(1, 1), &wsaData);

	if (err == SOCKET_ERROR)
	{
		cout << "WSAStartup() failed: " << GetLastError() << endl;
		return 1;
	}

	CTransport * UDP = new (nothrow) CUDP();
	CTransport * TCP = new (nothrow) CTCPMain();

	if (UDP->Bind() != S_OK)
	{
		return 1;
	}

	if (TCP->Bind() != S_OK)
	{
		return 1;
	}

	HANDLE m_hUdp = CreateThread(NULL, 0, UDPloop, UDP, 0, NULL);

	if (!m_hUdp)
	{
		cout << "Failed to create UDP thread" << endl;
		return 1;
	}

	HANDLE m_hTcp = CreateThread(NULL, 0, TCPloop, TCP, 0, NULL);

	if (!m_hTcp)
	{
		cout << "Failed to create main TCP thread" << endl;
		return 1;
	}

	HANDLE handles[2] = {UDP->m_hCloseEvent, TCP->m_hCloseEvent};

	::WaitForMultipleObjects(2, handles, FALSE, INFINITE);

	if (m_hUdp)
	{
		cout << "Closing UDP thread" << endl;
		::CloseHandle(m_hUdp);
		m_hUdp = NULL;
	}

	if (m_hTcp)
	{
		cout << "Closing main TCP thread" << endl;
		::CloseHandle(m_hTcp);
		m_hTcp = NULL;
	}

	WSACleanup();
    return 0;
}

