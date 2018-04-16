#include "tcp.h"
#include "udp.h"
#include <iphlpapi.h>
#include <fcntl.h>
#include "Message.h"
#include "global.h"

int getUdp(int flN, int flPid)
{
	PMIB_UDPTABLE_OWNER_MODULE pUdpTable;
	DWORD dwRetVal = 0;
	ULONG ulSize = sizeof(MIB_UDPTABLE_OWNER_MODULE);
	LPWSTR strBuff[10];

	pUdpTable = (PMIB_UDPTABLE_OWNER_MODULE)HeapAlloc(GetProcessHeap(), 0, ulSize);
	if (pUdpTable == NULL)
	{
		fflush(stdout);
		_setmode(_fileno(stdout), _O_U16TEXT);
		fwprintf(stdout, L"GetFormattedMessage failed with error # %ld\n", GetLastError());
		return 1;
	}

	if (dwRetVal = GetExtendedUdpTable(pUdpTable, &ulSize, TRUE, AF_INET, UDP_TABLE_OWNER_MODULE, 0) == ERROR_INSUFFICIENT_BUFFER)
	{
		HeapFree(GetProcessHeap(), 0, pUdpTable);
		pUdpTable = (PMIB_TCPTABLE2)HeapAlloc(GetProcessHeap(), 0, ulSize);
		if (pUdpTable == NULL)
		{
			fflush(stdout);
			_setmode(_fileno(stdout), _O_U16TEXT);
			fwprintf(stdout, L"GetFormattedMessage failed with error # %ld\n", GetLastError());
			return 1;
		}
	}

	if (dwRetVal = GetExtendedUdpTable(pUdpTable, &ulSize, TRUE, AF_INET, UDP_TABLE_OWNER_MODULE, 0) == NO_ERROR)
	{
		for (int counter = 0; counter < pUdpTable->dwNumEntries; counter++)
		{
			u_short localPortntoh = ntohs((u_short)pUdpTable->table[counter].dwLocalPort);
			pUdpTable->table[counter].dwLocalPort = (DWORD)localPortntoh;

			strBuff[0] = getFormattedMessageW(MESSAGE_NO_LINE_BREAKS, MSG_UDP, NULL);
			strBuff[1] = tranAddrtoIp(pUdpTable->table[counter].dwLocalAddr, localPortntoh, flN);
			strBuff[2] = L"*:*";
			if (flPid)
			{
				strBuff[3] = (LPWSTR)HeapAlloc(GetProcessHeap(), 0, 20);
				wsprintfW(strBuff[3], L"%d", (int)pUdpTable->table[counter].dwOwningPid);
				strBuff[4] = getFormattedMessageW(MESSAGE_LINE_BREAKS, MSG_UDP_FORMAT_WITH_PID, 10, strBuff[0], 30, strBuff[1], 45, strBuff[2],strBuff[3]);
			}
			else
			{
				strBuff[4] = getFormattedMessageW(MESSAGE_LINE_BREAKS, MSG_UDP_FORMAT, 10, strBuff[0], 30, strBuff[1],strBuff[2]);
			}
			outputMessageW(strBuff[4]);

			LocalFree(strBuff[0]);
			LocalFree(strBuff[1]);
			LocalFree(strBuff[2]);
			if (flPid)
			{
				HeapFree(GetProcessHeap(), 0, strBuff[3]);
			}
			LocalFree(strBuff[4]);
		}
	}

	HeapFree(GetProcessHeap(), 0, pUdpTable);

	return 0;
}