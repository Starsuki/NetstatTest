#include <stdio.h>
#include <io.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>
#include <fcntl.h>
#include <WinSnmp.h>
#include <conio.h>
#include "Message.h"
#include "global.h"
#include "general.h"
#include "tcp.h"
#include "udp.h"

/*��������������̬��*/
#pragma comment(lib,"iphlpapi.lib")
#pragma comment(lib,"ws2_32.lib")

int main(int argc, char *argv[])
{
	int flagA = 0;//-a���ܱ�־λ
	int flagNo = 0;//-n��-o���ܱ�־λ
	int sleepTime = 0;//CPU����ʱ��
	LPWSTR msgArgError;

	/*���������в���*/
	if (argc != 0)
	{
		for (int i = 1; i < argc; i++)
		{
			int argCount = 1;
			char arg = *(argv[i] + argCount);
			char argTime = *argv[i];

			if (argTime >= '0' && argTime <= '3')//��ʾ�������Ϊ0-3s��0s��ֻ��ʾһ��
			{
				sleepTime = (unsigned int)(argTime - '0');
				continue;
			}

			while (arg != 0)
			{
				if (arg == 0)
				{
					continue;
				}

				switch (toupper(arg))
				{
				case 65:
					flagA = 1;
					break;
				case 78:
					flagNo = flagNo | FLAG_N;
					break;
				case 79:
					flagNo = flagNo | FLAG_O;
					break;
				default:
					msgArgError = getFormattedMessageW(MESSAGE_LINE_BREAKS, MSG_ARG_ERROR, NULL);
					outputMessageW(msgArgError);
					LocalFree(msgArgError);
					return 0;
				}
				argCount++;
				arg = *(argv[i] + argCount);
			}

		}
	}
	LPWSADATA pWsaData;
	pWsaData = (LPWSADATA)HeapAlloc(GetProcessHeap(), 0, sizeof(WSADATA));
	if (WSAStartup(MAKEWORD(1, 1), pWsaData))
	{
		/*���������ʽ���������������������ݣ�������Ϊunicode�������*/
		fflush(stdout);//��׼�������ʹ��_acrt_iob_func()�����������п��õ�
		_setmode(_fileno(stdout), _O_U16TEXT);
		fwprintf(stdout, L"WSAStartup failed with error # %ld\n", GetLastError());
		return 1;
	}
	
	while (1)
	{
		if (flagNo & FLAG_O)
		{
			outputMessageW(getFormattedMessageW(MESSAGE_LINE_BREAKS, MSG_TITLE_WITH_PID, NULL));
		}
		else
		{
			outputMessageW(getFormattedMessageW(MESSAGE_LINE_BREAKS, MSG_TITLE, NULL));
		}
		getTcp(flagA, flagNo & FLAG_N, flagNo & FLAG_O);
		if (flagA)
		{
			getUdp(flagNo & FLAG_N, flagNo & FLAG_O);
		}

		if (!sleepTime)
		{
			break;
		}

		Sleep(sleepTime * 1000);
	}

	return 0;
}