#pragma once

class CTransport
{
public:
	virtual HRESULT Bind() = 0;
	virtual HRESULT Send() = 0;
	virtual HRESULT Receive() = 0;
	void Term();
protected:
	CTransport();
	~CTransport();
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


class CTCP : public CTransport
{
public:
	CTCP() {};
	CTCP(SOCKET sckt) { m_Socket = sckt; };
	~CTCP() {};
	HRESULT Bind() override;
	HRESULT Send() override;
	HRESULT Receive() override;
	HRESULT Listen();
};