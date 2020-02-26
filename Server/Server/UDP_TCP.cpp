#include "stdafx.h"
#include "UDP_TCP.h"

#define HALLOC( s ) ::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, s)
#define HREALLOC( p, s ) ::HeapReAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, p, s)
#define HFREE( p ) ::HeapFree(::GetProcessHeap(), 0u, p)
#define MAXPACKETSIZE 16 * 1024 // 16 KB

//EXAMPLE
//::WaitForSingleObject(m_hStartEvent, INFINITE);
//
//HANDLE m_hRTCPThread = (HANDLE)CreateNamedThread<g25::CRtsp20,
//	&g25::CRtsp20::StartRTCP>("RTCP_Thread", this, NULL, 0, 0, NULL);


CTransport::CTransport()
{
	m_pBuf = NULL;
	m_nBufLen = 0;
	//TODO: randomizer of ports
	m_nLocalPort = 10005;
}


CTransport::~CTransport()
{
	Term();
}


HRESULT CUDP::Bind()
{
	m_Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (m_Socket == INVALID_SOCKET)
	{
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
		return E_FAIL;
	}

	int time = 20000;  // 20 Secs Timeout 

	if (setsockopt(m_Socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&time, sizeof(time)) == SOCKET_ERROR)
	{
		return E_FAIL;
	}

	return S_OK;
}


HRESULT CTCP::Bind()
{
	m_Socket = socket(AF_INET, SOCK_STREAM, 0);

	if (m_Socket == INVALID_SOCKET)
	{
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
		return E_FAIL;
	}

	return S_OK;
}


HRESULT CUDP::Receive()
{
	fd_set fd = {1, m_Socket};
	timeval tv;
	tv.tv_sec = 20;
	FD_ZERO(&fd);
	FD_SET(m_Socket, &fd);

	int res = select(0, &fd, &fd, &fd, &tv);
	if (res == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		return E_FAIL;
	}

	char * temp = (char*)HALLOC(MAXPACKETSIZE);
	sockaddr_in SenderAddr;
	int SenderAddrSize = sizeof(SenderAddr);
	int actual_len = 0;

	actual_len = recvfrom(m_Socket, temp, MAXPACKETSIZE, 0, (SOCKADDR *)&SenderAddr, &SenderAddrSize);

	if ((actual_len == SOCKET_ERROR) || !actual_len)
	{
		int err = WSAGetLastError();
		HFREE(temp);
		temp = NULL;
		return err;
	}
	m_Client = SenderAddr;
	//TODO: if m_pBuf not NULL semaphor or something like it
	
	m_pBuf = (BYTE*)HALLOC(actual_len + 1);

	if (!m_pBuf)
	{
		HFREE(temp);
		temp = NULL;
		return E_OUTOFMEMORY;
	}

	memcpy(m_pBuf, temp, actual_len);
	m_nBufLen = actual_len;

	HFREE(temp);
	temp = NULL;

	return S_OK;
}


HRESULT CTCP::Receive()
{
	int actual_len = 0;
	char * temp = (char *)HALLOC(MAXPACKETSIZE);

	if (!temp)
	{
		return E_OUTOFMEMORY;
	}

	while (!actual_len)
	{
		actual_len = recv(m_Socket, temp, MAXPACKETSIZE, 0);
	}

	if (actual_len == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		HFREE(temp);
		temp = NULL;
		return E_FAIL;
	}

	m_pBuf = (BYTE*)HALLOC(actual_len + 1);

	if (!m_pBuf)
	{
		HFREE(temp);
		temp = NULL;
		return E_OUTOFMEMORY;
	}

	m_nBufLen = actual_len;
	memcpy(m_pBuf, temp, actual_len);

	HFREE(temp);
	temp = NULL;

	return S_OK;
}


HRESULT CUDP::Send()
{
	if (!m_pBuf || !m_nBufLen || m_nBufLen < 0)
	{
		return E_FAIL;
	}

	int len = sizeof(m_Client);

	int retlen = sendto(m_Socket, (char*)m_pBuf, m_nBufLen, 0, (SOCKADDR *)&m_Client, len);

	if (!retlen || (retlen == SOCKET_ERROR))
	{
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


HRESULT CTCP::Send()
{
	if (!m_pBuf || !m_nBufLen || m_nBufLen < 0)
	{
		return E_FAIL;
	}

	if (send(m_Socket, (char*)m_pBuf, m_nBufLen, 0) == SOCKET_ERROR)
	{
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
}


HRESULT CTCP::Listen()
{
	if (listen(m_Socket, SOMAXCONN) == SOCKET_ERROR)
	{
		return E_FAIL;
	}

	return S_OK;
}


//HRESULT CTCP::Temperary()
//{
//	while (1)
//	{
//		SOCKADDR_IN addr_c;
//		int addrlen = sizeof(addr_c);
//		SOCKET tcp = accept(m_Socket, (sockaddr*)&addr_c, &addrlen);
//
//		if (tcp == INVALID_SOCKET)
//		{
//			int a = WSAGetLastError();
//			continue;
//		}
//
//		/*if (connect(tcp, (sockaddr*)&addr_c, addrlen) == SOCKET_ERROR)
//		{
//			int a = WSAGetLastError();
//			return E_FAIL;
//		}*/
//
//		int time = 20000;  // 20 Secs Timeout 
//
//		if (setsockopt(tcp, SOL_SOCKET, SO_RCVTIMEO, (char *)&time, sizeof(time)) == SOCKET_ERROR)
//		{
//			return E_FAIL;
//		}
//
//		m_Socket = tcp;
//
//		while (1)
//		{
//			if (Receive() == S_OK)
//			{
//				Send();
//			}
//		}
//	}
//	return S_OK;
//}