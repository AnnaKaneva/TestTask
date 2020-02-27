#include "stdafx.h"
#include "UDP_TCP.h"
#include <iostream>


CTransport::CTransport()
{
	m_pBuf = NULL;
	m_nBufLen = 0;
	m_nServPort = 10005;
	m_pServIP = NULL;
	m_nLocalPort = 10001;
}


CTransport::~CTransport()
{
	Term();
}


HRESULT CUDP::Connection(const char * IP)
{
	if (strlen(IP) <= 0)
	{
		std::cout << "Incorrect arguments" << std::endl;
		return E_INVALIDARG;
	}

	if (m_pServIP)
	{
		HFREE(m_pServIP);
		m_pServIP = NULL;
	}

	m_pServIP = (char*)HALLOC(strlen(IP) + 1);

	if (!m_pServIP)
	{
		std::cout << "Couldn't allocate the memory" << std::endl;
		return E_OUTOFMEMORY;
	}

	memcpy(m_pServIP, IP, strlen(IP));

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


HRESULT CTCP::Connection(const char * IP)
{
	if (strlen(IP) <= 0)
	{
		std::cout << "Incorrect arguments" << std::endl;
		return E_INVALIDARG;
	}

	if (m_pServIP)
	{
		HFREE(m_pServIP);
		m_pServIP = NULL;
	}

	m_pServIP = (char*)HALLOC(strlen(IP) + 1);

	if (!m_pServIP)
	{
		std::cout << "Couldn't allocate the memory" << std::endl;
		return E_OUTOFMEMORY;
	}

	memcpy(m_pServIP, IP, strlen(IP));

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

	sockaddr_in addr;
	ZeroMemory(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = inet_addr(m_pServIP);
	addr.sin_port = htons((u_short)m_nServPort);

	if (connect(m_Socket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		int a = WSAGetLastError();
		std::cout << "An error occured while connecting: " << a << std::endl;
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
		std::cout << "An error occured while receiving: " << err << std::endl;
		HFREE(temp);
		temp = NULL;
		return E_FAIL;
	}

	if (SenderAddr.sin_addr.S_un.S_addr != inet_addr(m_pServIP))
	{
		HFREE(temp);
		temp = NULL;
		std::cout << "Received packet fron unknown destination" << std::endl;
		return E_FAIL;
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

	memcpy(m_pBuf, temp, actual_len);
	m_nBufLen = actual_len;

	HFREE(temp);
	temp = NULL;

	std::cout << "Return from the server: " << (char*)m_pBuf << std::endl;

	if (m_pBuf)
	{
		HFREE(m_pBuf);
		m_pBuf = NULL;
		m_nBufLen = 0;
	}

	return S_OK;
}


HRESULT CTCP::Receive()
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
		std::cout << "An error occured while setting timer for receiving: " << err << std::endl;
		return E_FAIL;
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

	HFREE(temp);
	temp = NULL;

	std::cout << "Return from the server: " << (char*)m_pBuf << std::endl;

	if (m_pBuf)
	{
		HFREE(m_pBuf);
		m_pBuf = NULL;
		m_nBufLen = 0;
	}

	return S_OK;
}


HRESULT CUDP::Send(BYTE * buf, const int len)
{
	if (!buf || !len || len < 0)
	{
		std::cout << "Incorrect arguments" << std::endl;
		return E_INVALIDARG;
	}

	sockaddr_in addr;
	ZeroMemory(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = inet_addr(m_pServIP);
	addr.sin_port = htons((u_short)m_nServPort);
	int AddrSize = sizeof(addr);

	int retlen = sendto(m_Socket, (char*)(buf), len, 0, (SOCKADDR *)&addr, AddrSize);

	if (!retlen || (retlen == SOCKET_ERROR))
	{
		std::cout << "An error occured while setting timer for sending: " << WSAGetLastError() << std::endl;
		return E_FAIL;
	}

	return S_OK;
}


HRESULT CTCP::Send(BYTE * buf, const int len)
{
	if (!buf || !len || len < 0)
	{
		std::cout << "Incorrect arguments" << std::endl;
		return E_INVALIDARG;
	}

	int retlen = send(m_Socket, (char*)buf, len, 0);

	if ((retlen == SOCKET_ERROR) || !retlen)
	{
		std::cout << "An error occured while setting timer for sending: " << WSAGetLastError() << std::endl;
		return E_FAIL;
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

	if (m_pServIP)
	{
		HFREE(m_pServIP);
		m_pServIP = NULL;
	}
}