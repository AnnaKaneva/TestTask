// Server.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "UDP_TCP.h"


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
				return 1;
			}
		}
		else if (err != WSAETIMEDOUT)
		{
			::SetEvent(((CUDP*)transp)->m_hCloseEvent);
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
		printf("WSAStartup() failed: %ld\n", GetLastError());
		return 1;
	}

	CTransport * UDP = new CUDP();
	CTransport * TCP = new CTCPMain();

	UDP->Bind();
	TCP->Bind();

	HANDLE m_hUdp = CreateThread(NULL, 0, UDPloop, UDP, 0, NULL);

	if (!m_hUdp)
	{
		return 1;
	}

	HANDLE m_hTcp = CreateThread(NULL, 0, TCPloop, TCP, 0, NULL);

	if (!m_hTcp)
	{
		return 1;
	}

	HANDLE handles[2] = {UDP->m_hCloseEvent, TCP->m_hCloseEvent};

	::WaitForMultipleObjects(2, handles, FALSE, INFINITE);

	if (m_hUdp)
	{
		::CloseHandle(m_hUdp);
		m_hUdp = NULL;
	}

	if (m_hTcp)
	{
		::CloseHandle(m_hTcp);
		m_hTcp = NULL;
	}

	WSACleanup();
    return 0;
}

