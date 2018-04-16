#ifndef _GENERAL_H
#define _GENERAL_H

#include <WinSock2.h>

void outputMessageW(LPWSTR buffer);
LPWSTR getFormattedMessageW(int noNewLine, int messageId, ...);
wchar_t * my_vsnwprintf(size_t size, wchar_t *format, ...);

#endif // _GENERAL_H

