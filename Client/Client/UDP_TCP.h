#pragma once

class CTransport
{
public:
	virtual HRESULT Connection(const char * IP) = 0;
	virtual HRESULT Send(BYTE * buf, const int len) = 0;
	virtual HRESULT Receive() = 0;
	void Term();
protected:
	CTransport();
	~CTransport();
	SOCKET m_Socket;
	char * m_pServIP;
	int m_nServPort;
	int m_nLocalPort;
	BYTE * m_pBuf;
	int m_nBufLen;
};

class CUDP : public CTransport
{
public:
	CUDP() {};
	~CUDP() {};
	HRESULT Connection(const char * IP) override;
	HRESULT Send(BYTE * buf, const int len) override;
	HRESULT Receive() override;
};


class CTCP : public CTransport
{
public:
	CTCP() {};
	~CTCP() {};
	HRESULT Connection(const char * IP) override;
	HRESULT Send(BYTE * buf, const int len) override;
	HRESULT Receive() override;
};