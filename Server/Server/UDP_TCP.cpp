#include "stdafx.h"
#include "UDP_TCP.h"
#include <iostream>

#define HALLOC( s ) ::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, s)
#define HREALLOC( p, s ) ::HeapReAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, p, s)
#define HFREE( p ) ::HeapFree(::GetProcessHeap(), 0u, p)
#define MAXPACKETSIZE 16 * 1024 // 16 KB


CTransport::CTransport()
{
	m_pBuf = NULL;
	m_nBufLen = 0;
	m_nLocalPort = 10005;
	m_hCloseEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}


CTCPConn::CTCPConn(SOCKET sckt, sockaddr_in addr)
{
	m_Socket = sckt;
	m_Client = addr;
}


CTransport::~CTransport()
{
	Term();
}


CTCPMain::~CTCPMain()
{
	for (int i = 0; i < m_pVecConn.size(); i++)
	{
		if (m_pVecConn[i].first)
		{
			::CloseHandle(m_pVecConn[i].first);
			m_pVecConn[i].first = NULL;
		}

		delete(m_pVecConn[i].second);
	}
	m_pVecConn.clear();
}


HRESULT CUDP::Bind()
{
	m_Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (m_Socket == INVALID_SOCKET)
	{
		std::cout << "Couldn't create a socket: " << WSAGetLastError() << std::endl;
		return E_FAIL;
	}

	sockaddr_in addrMy;
	ZeroMemory(&addrMy, sizeof(addrMy));
	addrMy.sin_family = AF_INET;
	addrMy.sin_addr.S_un.S_addr = inet_addr("192.168.1.70");//local IP

	addrMy.sin_port = htons((u_short)m_nLocalPort);

	if (bind(m_Socket, (SOCKADDR*)&addrMy, sizeof(addrMy)) == SOCKET_ERROR)
	{
		int a = WSAGetLastError();
		std::cout << "An error occured while binding: " << a << std::endl;
		return E_FAIL;
	}

	int time = 20000;  // 20 Secs Timeout 

	if (setsockopt(m_Socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&time, sizeof(time)) == SOCKET_ERROR)
	{
		std::cout << "An error occured while setting timer for receiving: " << WSAGetLastError() << std::endl;
		return E_FAIL;
	}

	if (setsockopt(m_Socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&time, sizeof(time)) == SOCKET_ERROR)
	{
		std::cout << "An error occured while setting timer for sending: " << WSAGetLastError() << std::endl;
		return E_FAIL;
	}

	return S_OK;
}


HRESULT CTCPMain::Bind()
{
	m_Socket = socket(AF_INET, SOCK_STREAM, 0);

	if (m_Socket == INVALID_SOCKET)
	{
		std::cout << "Couldn't create a socket: " << WSAGetLastError() << std::endl;
		return E_FAIL;
	}

	sockaddr_in addrMy;
	ZeroMemory(&addrMy, sizeof(addrMy));
	addrMy.sin_family = AF_INET;
	addrMy.sin_addr.S_un.S_addr = inet_addr("192.168.1.70");//local IP

	addrMy.sin_port = htons((u_short)m_nLocalPort);

	if (bind(m_Socket, (SOCKADDR*)&addrMy, sizeof(addrMy)) == SOCKET_ERROR)
	{
		int a = WSAGetLastError();
		std::cout << "An error occured while binding: " << a << std::endl;
		return E_FAIL;
	}

	if (listen(m_Socket, SOMAXCONN) == SOCKET_ERROR)
	{
		std::cout << "An error occured while listening: " << WSAGetLastError() << std::endl;
		return E_FAIL;
	}

	return S_OK;
}


HRESULT CTCPConn::Bind()
{
	int time = 20000;  // 20 Secs Timeout  

	if (setsockopt(m_Socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&time, sizeof(time)) == SOCKET_ERROR)
	{
		std::cout << "An error occured while setting timer for receiving: " << WSAGetLastError() << std::endl;
		return E_FAIL;
	}

	if (setsockopt(m_Socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&time, sizeof(time)) == SOCKET_ERROR)
	{
		std::cout << "An error occured while setting timer for sending: " << WSAGetLastError() << std::endl;
		return E_FAIL;
	}

	return S_OK;
}


HRESULT CUDP::Receive()
{
	char * temp = (char*)HALLOC(MAXPACKETSIZE);

	if (!temp)
	{
		std::cout << "Couldn't allocate the memory" << std::endl;
		return E_OUTOFMEMORY;
	}

	sockaddr_in SenderAddr;
	int SenderAddrSize = sizeof(SenderAddr);
	int actual_len = 0;

	actual_len = recvfrom(m_Socket, temp, MAXPACKETSIZE, 0, (SOCKADDR *)&SenderAddr, &SenderAddrSize);

	if ((actual_len == SOCKET_ERROR) || !actual_len)
	{
		int err = WSAGetLastError();
		HFREE(temp);
		temp = NULL;
		if (err != WSAETIMEDOUT)
		{
			std::cout << "An error occured while receiving: " << err << std::endl;
		}
		return err;
	}
	m_Client = SenderAddr;

	if (m_pBuf)
	{
		HFREE(m_pBuf);
		m_pBuf = NULL;
	}
	
	m_pBuf = (BYTE*)HALLOC(actual_len + 1);

	if (!m_pBuf)
	{
		HFREE(temp);
		temp = NULL;
		std::cout << "Couldn't allocate the memory" << std::endl;
		return E_OUTOFMEMORY;
	}

	memcpy(m_pBuf, temp, actual_len);
	m_nBufLen = actual_len;

	std::cout << "Received through UDP: " << m_pBuf << std::endl;

	HFREE(temp);
	temp = NULL;

	return S_OK;
}


HRESULT CTCPConn::Receive()
{
	int actual_len = 0;
	char * temp = (char *)HALLOC(MAXPACKETSIZE);

	if (!temp)
	{
		std::cout << "Couldn't allocate the memory" << std::endl;
		return E_OUTOFMEMORY;
	}

	actual_len = recv(m_Socket, temp, MAXPACKETSIZE, 0);

	if ((actual_len == SOCKET_ERROR) || !actual_len)
	{
		int err = WSAGetLastError();
		HFREE(temp);
		temp = NULL;
		if (err != WSAETIMEDOUT)
		{
			std::cout << "An error occured while receiving: " << err << std::endl;
		}
		return err;
	}

	if (m_pBuf)
	{
		HFREE(m_pBuf);
		m_pBuf = NULL;
	}

	m_pBuf = (BYTE*)HALLOC(actual_len + 1);

	if (!m_pBuf)
	{
		HFREE(temp);
		temp = NULL;
		std::cout << "Couldn't allocate the memory" << std::endl;
		return E_OUTOFMEMORY;
	}

	m_nBufLen = actual_len;
	memcpy(m_pBuf, temp, actual_len);

	std::cout << "Received through TCP: " << m_pBuf << std::endl;

	HFREE(temp);
	temp = NULL;

	return S_OK;
}


HRESULT CUDP::Send()
{
	if (!m_pBuf || !m_nBufLen || m_nBufLen < 0)
	{
		std::cout << "Incorrect arguments" << std::endl;
		return E_INVALIDARG;
	}

	int len = sizeof(m_Client);

	int retlen = sendto(m_Socket, (char*)m_pBuf, m_nBufLen, 0, (SOCKADDR *)&m_Client, len);

	if (!retlen || (retlen == SOCKET_ERROR))
	{
		std::cout << "An error occured while sending: " << WSAGetLastError() << std::endl;
		return E_FAIL;
	}

	if (m_pBuf)
	{
		HFREE(m_pBuf);
		m_pBuf = NULL;
		m_nBufLen = 0;
	}

	return S_OK;
}


HRESULT CTCPConn::Send()
{
	if (!m_pBuf || !m_nBufLen || m_nBufLen < 0)
	{
		std::cout << "Incorrect arguments" << std::endl;
		return E_INVALIDARG;
	}

	int retlen = send(m_Socket, (char*)m_pBuf, m_nBufLen, 0);

	if (( retlen == SOCKET_ERROR) || !retlen)
	{
		std::cout << "An error occured while sending: " << WSAGetLastError() << std::endl;
		return E_FAIL;
	}

	if (m_pBuf)
	{
		HFREE(m_pBuf);
		m_pBuf = NULL;
		m_nBufLen = 0;
	}

	return S_OK;
}


void CTransport::Term()
{
	shutdown(m_Socket, SD_BOTH);
	closesocket(m_Socket);

	if (m_pBuf)
	{
		HFREE(m_pBuf);
		m_pBuf = NULL;
		m_nBufLen = 0;
	}

	if (m_hCloseEvent)
	{
		::CloseHandle(m_hCloseEvent);
		m_hCloseEvent = NULL;
	}
}


DWORD WINAPI ConnectedTCP(LPVOID transp)
{
	CTCPConn * transpC = (CTCPConn*)transp;

	if (transpC->Bind() != S_OK)
	{
		return 1;
	}

	for (;;)
	{
		int err = 0;

		if ((err = transpC->Receive()) == S_OK)
		{
			if (transpC->Send() != S_OK)
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


DWORD WINAPI CTCPMain::Accept()
{
	for (;;)
	{
		for (int i = 0; i < m_pVecConn.size(); i++)
		{
			DWORD code;
			int res = GetExitCodeThread(m_pVecConn[i].first, &code);

			if (!res)
			{
				::SetEvent(m_hCloseEvent);
				std::cout << "Setting closing event in TCP thread" << std::endl;
				return 1;
			}

			if (code != STILL_ACTIVE)
			{
				if (m_pVecConn[i].first)
				{
					std::cout << "Closing connected TCP thread" << std::endl;
					::CloseHandle(m_pVecConn[i].first);
					m_pVecConn[i].first = NULL;
				}

				delete(m_pVecConn[i].second);

				m_pVecConn.erase(m_pVecConn.begin() + i);
				i--;
			}
		}

		fd_set fd = { 1, m_Socket };
		timeval tv;
		tv.tv_sec = 20;
		FD_ZERO(&fd);
		FD_SET(m_Socket, &fd);

		int res = select(0, &fd, NULL, NULL, &tv);
		if (res == SOCKET_ERROR)
		{
			int err = WSAGetLastError();
			::SetEvent(m_hCloseEvent);
			std::cout << "Setting closing event in TCP thread" << std::endl;
			return E_FAIL;
		}

		if (!res)
		{
			continue;
		}

		SOCKADDR_IN addr_c;
		int addrlen = sizeof(addr_c);
		SOCKET tcp = accept(m_Socket, (sockaddr*)&addr_c, &addrlen);
		
		if (tcp == INVALID_SOCKET)
		{
			int a = WSAGetLastError();
			std::cout << "Couldn't create connected socket: " << a << std::endl;
			//::SetEvent(m_hCloseEvent);
			//return 1;
			continue;
		}

		CTransport * tr = new(std::nothrow) CTCPConn(tcp, addr_c);

		//createthread
		HANDLE h = CreateThread(NULL, 0, ConnectedTCP, tr, 0, NULL);

		if (!h)
		{
			::SetEvent(m_hCloseEvent);
			std::cout << "Failed to create connected TCP thread" << std::endl;
			std::cout << "Setting closing event in TCP thread" << std::endl;
			return 1;
		}

		m_pVecConn.push_back(std::make_pair(h, tr));
	}
}