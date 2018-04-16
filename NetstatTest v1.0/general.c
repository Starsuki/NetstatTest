#include "general.h"
#include <io.h>
#include <stdio.h>
#include <fcntl.h>
#include <WS2tcpip.h>

void outputMessageW(LPWSTR buffer)
{
	if (buffer)
	{
		fflush(stdout);
		_setmode(_fileno(stdout), _O_U16TEXT);
		fwprintf(stdout, L"%s", buffer);
	}
}

/*
生成格式化消息，和LocalFree()配套使用，
分为考虑参数中的换行符和不考虑换行符两种情况
*/
LPWSTR getFormattedMessageW(int noNewLine, int messageId, ...)
{
	LPWSTR pMessBuf = NULL;
	va_list args;
	va_start(args, messageId);

	if (noNewLine)
	{
		if (FormatMessageW(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_MAX_WIDTH_MASK,
			NULL,
			messageId,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPWSTR)&pMessBuf,
			0,
			&args))
		{
			va_end(args);
			return pMessBuf;
		}
	}

	if (FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
		NULL,
		messageId,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&pMessBuf,
		0,
		&args))
	{
		va_end(args);
		return pMessBuf;
	}

	va_end(args);
	DWORD lastError = GetLastError();

	FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		lastError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&pMessBuf,
		0,
		NULL);

	outputMessageW(pMessBuf);
	LocalFree(pMessBuf);

	return NULL;
}

/*向缓冲区中格式化输出字符串*/
wchar_t * my_vsnwprintf(size_t size, wchar_t *format, ...)
{
	wchar_t *buffer = (wchar_t *)HeapAlloc(GetProcessHeap(), 0, size * 2);
	va_list args;
	va_start(args, format);
	int dwRetVal = _vsnwprintf(buffer, size, format, args);
	va_end(args);

	if (buffer)
	{
		return buffer;
	}
	else
	{
		fflush(stdout);
		_setmode(_fileno(stdout), _O_U16TEXT);
		fwprintf(stdout, L"Error.");
	}

	return NULL;
}