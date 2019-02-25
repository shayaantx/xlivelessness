#pragma once

typedef struct {
	XNDNS* resultOut;
	HANDLE onDnsLookupCompletedEvent;
	PDNS_QUERY_CANCEL cancel;
} DNS_REQUEST, *PDNS_REQUEST;

class DnsHelper {
public:
	int lookup(const char *hostname, WSAEVENT onDnsLookupCompletedEvent, XNDNS** ppxndns);
	int release(XNDNS* ppxndns);
};