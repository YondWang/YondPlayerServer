#pragma once
#include "Socket.h"
#include "http_parser.h"
#include <map>

class CHttpParser
{
private:
	http_parser m_parser;
	http_parser_settings m_settings;
	std::map<Buffer, Buffer> m_HeaderValues;
	Buffer m_status;
	Buffer m_url;
	Buffer m_body;
	bool m_complete;
	Buffer m_lastField;
public:
	CHttpParser();
	~CHttpParser();
	CHttpParser(const CHttpParser& http);
	CHttpParser& operator=(const CHttpParser& http);

public:
	size_t Parser(const Buffer& data);
	unsigned Method() const	//GET POST ... ²Î¿¼http_parser.h HTTP_METHODºê
	{
		return m_parser.method;
	}
	const std::map<Buffer, Buffer>& headers() { return m_HeaderValues; }
	const Buffer& Status() const { return m_status; }
	const Buffer& Url() const { return m_url; }
	const Buffer& Body() const { return m_body; }
	unsigned Errno() const { return m_parser.http_errno; }

protected:
	static int OnMessageBegin(http_parser* parser);
	static int OnUrl(http_parser* parser, const char* at, size_t length);
	static int OnStatus(http_parser* parser, const char* at, size_t length);
	static int OnHeaderField(http_parser* parser, const char* at, size_t length);
	static int OnHeaderValue(http_parser* parser, const char* at, size_t length);

	static int OnHeadersComplete(http_parser* parser);
	static int OnBody(http_parser* parser, const char* at, size_t length);

	static int OnMessageComplete(http_parser* parser);

	int OnMessageBegin();
	int OnUrl(const char* at, size_t length);
	int OnStatus(const char* at, size_t length);
	int OnHeaderField(const char* at, size_t length);
	int OnHeaderValue(const char* at, size_t length);
	int OnHeadersComplete();
	int OnBody(const char* at, size_t length);
	int OnMessageComplete();
};

