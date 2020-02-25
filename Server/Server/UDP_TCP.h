#pragma once

class CTransport
{
public:
	virtual HRESULT Listen() = 0;
	virtual HRESULT Send() = 0;
	virtual HRESULT Receive() = 0;
	virtual HRESULT Temperary() = 0;
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
	HRESULT Listen() override;
	HRESULT Send() override;
	HRESULT Receive() override;
};


class CTCP : public CTransport
{
public:
	CTCP() {};
	~CTCP() {};
	HRESULT Listen() override;
	HRESULT Send() override;
	HRESULT Receive() override;
	HRESULT Temperary() override;
};