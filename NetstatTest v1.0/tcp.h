#ifndef _TCP_H
#define _TCP_H

#include<stdio.h>
#include <WS2tcpip.h>

wchar_t *tranAddrtoIp(DWORD dwAddr, DWORD dwPort, int flag);
int getTcp2(int flA, int flN, int flPid);

#endif // !_TCP_H
