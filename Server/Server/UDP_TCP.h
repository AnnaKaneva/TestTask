#pragma once
#include <vector>
#include <utility>

class CTransport
{
public:
	CTransport();
	~CTransport();
	virtual HRESULT Bind() = 0;
	virtual HRESULT Send() = 0;
	virtual HRESULT Receive() = 0;
	void Term();
	HANDLE m_hCloseEvent;
protected:
	SOCKET m_Socket;
	sockaddr_in m_Client;
	int m_nLocalPort;
	BYTE * m_pBuf;
	int m_nBufLen;
};

class CUDP : public CTransport
{
public:
	CUDP() {};
	~CUDP() {};
	HRESULT Bind() override;
	HRESULT Send() override;
	HRESULT Receive() override;
};


class CTCPConn : public CTransport
{
public:
	CTCPConn(SOCKET sckt, sockaddr_in addr);
	~CTCPConn() {};
	HRESULT Bind() override;
	HRESULT Send() override;
	HRESULT Receive() override;
};


class CTCPMain : public CTransport
{
public:
	CTCPMain() {};
	~CTCPMain();
	HRESULT Bind() override;
	HRESULT Send() override { return E_FAIL; };
	HRESULT Receive() override { return E_FAIL; };
	DWORD WINAPI Accept();
	std::vector <std::pair<HANDLE, CTransport*>> m_pVecConn;
};