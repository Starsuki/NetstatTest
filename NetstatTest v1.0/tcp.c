#include "tcp.h"
#include <iphlpapi.h>
#include <fcntl.h>
#include "Message.h"
#include "global.h"

/*IPv4*/
wchar_t *tranAddrtoIp(DWORD dwAddr, DWORD dwPort, int flag)
{
	struct sockaddr_in nodeInfo;
	char hostName[NI_MAXHOST];
	char servName[NI_MAXSERV];
	DWORD dwRetVal;

	nodeInfo.sin_family = AF_INET;
	nodeInfo.sin_addr.S_un.S_addr = dwAddr;
	nodeInfo.sin_port = htons((u_short)dwPort);//HBO -> NBO

	/*
	获取主机IP，
	分为不使用别名和使用别名两种情况
	*/
	if (flag)
	{
		dwRetVal = GetNameInfoW((struct sockaddr *)&nodeInfo,
			sizeof(struct sockaddr),
			hostName,
			NI_MAXHOST,
			0,
			0,
			NI_NOFQDN | NI_NUMERICHOST | NI_NUMERICSERV);
	}
	else
	{
		dwRetVal = GetNameInfoW((struct sockaddr *)&nodeInfo,
			sizeof(struct sockaddr),
			hostName,
			NI_MAXHOST,
			0,
			0,
			NI_NOFQDN);
	}

	if (dwRetVal != 0)
	{
		fflush(stdout);
		_setmode(_fileno(stdout), _O_U16TEXT);
		fwprintf(stdout, L"Getnameinfo failed with error # %ld\n", WSAGetLastError());
		return NULL;
	}

	/*
	获取主机相应TCP连接端口号，
	分为不使用别名和使用别名两种情况
	*/
	if (flag)
	{
		dwRetVal = GetNameInfoW((struct sockaddr *)&nodeInfo,
			sizeof(struct sockaddr),
			0,
			0,
			servName,
			NI_MAXSERV,
			NI_NOFQDN | NI_NUMERICHOST | NI_NUMERICSERV);
	}
	else
	{
		dwRetVal = GetNameInfoW((struct sockaddr *)&nodeInfo,
			sizeof(struct sockaddr),
			0,
			0,
			servName,
			NI_MAXSERV,
			NI_NOFQDN);
	}

	if (dwRetVal != 0)
	{
		fflush(stdout);
		_setmode(_fileno(stdout), _O_U16TEXT);
		fwprintf(stdout, L"Getnameinfo failed with error # %ld\n", WSAGetLastError());
		return NULL;
	}

	/*拼接为“IP：端口号”的格式*/
	int nodeNameLen = lstrlenW(hostName) + lstrlenW(servName) + 2;
	size_t buffSize = nodeNameLen;
	wchar_t *format = L"%s:%s";
	return my_vsnwprintf(buffSize, format, hostName, servName);
}

int getTcp(int flA, int flN, int flPid)
{
	PMIB_TCPTABLE2 pTcpTable2;
	DWORD dwRetVal = 0;
	ULONG ulSize = sizeof(MIB_TCPTABLE2);
	LPWSTR strBuff[10];

	pTcpTable2 = (PMIB_TCPTABLE2)HeapAlloc(GetProcessHeap(), 0, ulSize);
	if (pTcpTable2 == NULL)
	{
		fflush(stdout);
		_setmode(_fileno(stdout), _O_U16TEXT);
		fwprintf(stdout, L"GetFormattedMessage failed with error # %ld\n", GetLastError());
		return 1;
	}

	/*
	若ulSize大小不足以存放tcptable，则GetTcpTable2()返回ERROR_INSUFFICIENT_BUFFER，
	ulSize被赋予一个足以存放tcptable的值
	*/
	if (dwRetVal = GetTcpTable2(pTcpTable2, &ulSize, TRUE) == ERROR_INSUFFICIENT_BUFFER)
	{
		HeapFree(GetProcessHeap(), 0, pTcpTable2);
		pTcpTable2 = (PMIB_TCPTABLE2)HeapAlloc(GetProcessHeap(), 0, ulSize);
		if (pTcpTable2 == NULL)
		{
			fflush(stdout);
			_setmode(_fileno(stdout), _O_U16TEXT);
			fwprintf(stdout, L"GetFormattedMessage failed with error # %ld\n", GetLastError());
			return 1;
		}
	}

	/*第一次可能获取失败，再获取一次*/
	if (dwRetVal = GetTcpTable2(pTcpTable2, &ulSize, TRUE) == NO_ERROR)
	{
		for (int counter = 0; counter < pTcpTable2->dwNumEntries; counter++)
		{
			if (!flA)
			{
				if (pTcpTable2->table[counter].dwState == MIB_TCP_STATE_LISTEN)
				{
					continue;
				}
			}

			/*端口号NBO->HBO*/
			u_short localPortntoh = ntohs((u_short)pTcpTable2->table[counter].dwLocalPort);
			pTcpTable2->table[counter].dwLocalPort = (DWORD)localPortntoh;
			u_short remotePortntoh = ntohs((u_short)pTcpTable2->table[counter].dwRemotePort);
			pTcpTable2->table[counter].dwRemotePort = (DWORD)remotePortntoh;

			strBuff[0] = getFormattedMessageW(MESSAGE_NO_LINE_BREAKS, MSG_TCP, NULL);
			strBuff[1] = tranAddrtoIp(pTcpTable2->table[counter].dwLocalAddr, localPortntoh, flN);
			strBuff[2] = tranAddrtoIp(pTcpTable2->table[counter].dwRemoteAddr, remotePortntoh, flN);
			switch (pTcpTable2->table[counter].dwState)
			{
			case MIB_TCP_STATE_CLOSED:
				strBuff[3] = getFormattedMessageW(MESSAGE_NO_LINE_BREAKS, MSG_CLOSED, NULL);
				break;
			case MIB_TCP_STATE_LISTEN:
				strBuff[3] = getFormattedMessageW(MESSAGE_NO_LINE_BREAKS, MSG_LISTEN, NULL);
				break;
			case MIB_TCP_STATE_SYN_SENT:
				strBuff[3] = getFormattedMessageW(MESSAGE_NO_LINE_BREAKS, MSG_SYN_SENT, NULL);
				break;
			case MIB_TCP_STATE_SYN_RCVD:
				strBuff[3] = getFormattedMessageW(MESSAGE_NO_LINE_BREAKS, MSG_SYN_RCVD, NULL);
				break;
			case MIB_TCP_STATE_ESTAB:
				strBuff[3] = getFormattedMessageW(MESSAGE_NO_LINE_BREAKS, MSG_ESTAB, NULL);
				break;
			case MIB_TCP_STATE_FIN_WAIT1:
				strBuff[3] = getFormattedMessageW(MESSAGE_NO_LINE_BREAKS, MSG_FIN_WAIT1, NULL);
				break;
			case MIB_TCP_STATE_FIN_WAIT2:
				strBuff[3] = getFormattedMessageW(MESSAGE_NO_LINE_BREAKS, MSG_FIN_WAIT2, NULL);
				break;
			case MIB_TCP_STATE_CLOSE_WAIT:
				strBuff[3] = getFormattedMessageW(MESSAGE_NO_LINE_BREAKS, MSG_CLOSE_WAIT, NULL);
				break;
			case MIB_TCP_STATE_CLOSING:
				strBuff[3] = getFormattedMessageW(MESSAGE_NO_LINE_BREAKS, MSG_CLOSING, NULL);
				break;
			case MIB_TCP_STATE_LAST_ACK:
				strBuff[3] = getFormattedMessageW(MESSAGE_NO_LINE_BREAKS, MSG_LAST_ACK, NULL);
				break;
			case MIB_TCP_STATE_TIME_WAIT:
				strBuff[3] = getFormattedMessageW(MESSAGE_NO_LINE_BREAKS, MSG_TIME_WAIT, NULL);
				break;
			case MIB_TCP_STATE_DELETE_TCB:
				strBuff[3] = getFormattedMessageW(MESSAGE_NO_LINE_BREAKS, MSG_DELETE_TCB, NULL);
				break;
			}
			if (flPid)
			{
				strBuff[4] = (LPWSTR)HeapAlloc(GetProcessHeap(), 0, 20);
				wsprintfW(strBuff[4], L"%d", (int)pTcpTable2->table[counter].dwOwningPid);
				strBuff[5] = getFormattedMessageW(MESSAGE_LINE_BREAKS, MSG_TCP_FORMAT_WITH_PID, 10, strBuff[0], 30, strBuff[1], 30, strBuff[2], 15, strBuff[3], strBuff[4]);
			}
			else
			{
				strBuff[5] = getFormattedMessageW(MESSAGE_LINE_BREAKS, MSG_TCP_FORMAT, 10, strBuff[0], 30, strBuff[1], 30, strBuff[2], strBuff[3]);
			}
			outputMessageW(strBuff[5]);

			LocalFree(strBuff[0]);
			LocalFree(strBuff[1]);
			LocalFree(strBuff[2]);
			LocalFree(strBuff[3]);
			if (flPid)
			{
				HeapFree(GetProcessHeap(), 0, strBuff[4]);
			}
			LocalFree(strBuff[5]);
		}
	}

	HeapFree(GetProcessHeap(), 0, pTcpTable2);

	return 0;
}