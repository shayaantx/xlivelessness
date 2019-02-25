#include <winsock2.h>
#include <WinDNS.h>
#include "xdefs.h"
#include "dnshelper.h"

void WINAPI dnsQueryCompletedHandler(PVOID queryContext, PDNS_QUERY_RESULT dnsQueryResults) {
	DNS_REQUEST* dnsRequest = static_cast<DNS_REQUEST*>(queryContext);
	dnsRequest->resultOut->iStatus = WSAEINPROGRESS;
	//TODO: handle different dns statuses
	if (dnsQueryResults->QueryStatus == ERROR_SUCCESS) {
		dnsRequest->resultOut->iStatus = 0;
		int count = 0;
		IN_ADDR aina[8];
		for (PDNS_RECORD record = dnsQueryResults->pQueryRecords; record; record = record->pNext) {
			switch (record->wType) {
			case DNS_TYPE_A:
				struct in_addr addr;
				addr.s_addr = (long)record->Data.A.IpAddress;
				dnsRequest->resultOut->iStatus = 0;
				dnsRequest->resultOut->aina[count] = addr;
				count++;
				break;
			}
		}

		dnsRequest->resultOut->cina = count;
		DnsRecordListFree(dnsQueryResults->pQueryRecords, DnsFreeRecordList);
	}
	else {
		//TODO: log dns query failed with status
		dnsRequest->resultOut->iStatus = 1;
	}
	if (!WSASetEvent(dnsRequest->onDnsLookupCompletedEvent)) {
		int error = WSAGetLastError();
		//TODO: log error
	}

	delete dnsRequest->cancel;
	delete dnsRequest;
}

//TODO: support async cancel
int DnsHelper::lookup(const char * hostname, WSAEVENT onDnsLookupCompletedEvent, XNDNS ** ppxndns)
{
	DNS_REQUEST* dnsRequest = new DNS_REQUEST;
	DNS_QUERY_REQUEST dnsQueryRequest = { 0 };
	DNS_QUERY_RESULT dnsQueryResults = { 0 };
	dnsRequest->cancel = { 0 };

	dnsQueryRequest.Version = DNS_QUERY_REQUEST_VERSION1;
	dnsQueryResults.Version = DNS_QUERY_REQUEST_VERSION1;
	wchar_t wHostname[512];
	mbstowcs(wHostname, hostname, 512);
	dnsQueryRequest.QueryName = wHostname;
	dnsQueryRequest.QueryType = DNS_TYPE_A;
	dnsQueryRequest.QueryOptions = DNS_QUERY_STANDARD;
	dnsQueryRequest.pQueryCompletionCallback = dnsQueryCompletedHandler;
	dnsQueryRequest.pQueryContext = dnsRequest;

	dnsRequest->resultOut = *ppxndns;
	dnsRequest->onDnsLookupCompletedEvent = onDnsLookupCompletedEvent;

	DNS_STATUS status = DnsQueryEx(&dnsQueryRequest, &dnsQueryResults, dnsRequest->cancel);
	if (status != DNS_REQUEST_PENDING) {
		//TODO: log
		//TODO: check other dns statuses
		delete dnsRequest->cancel;
		delete dnsRequest;
		return status;
	}
	return NO_ERROR;
}

int DnsHelper::release(XNDNS* ppxndns)
{
	delete ppxndns;
	return 0;
}
