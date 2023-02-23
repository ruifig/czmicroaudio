//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// 
//
#include "czStandardC.h"


#include "czPlatformStableHeaders.h"

//#include "czThread.h"
#include "czFunctional.h"
#include "czMutex.h"
#include "czConditionVariable.h"


#if 0
void* memset(void *dest, int c, size_t count)
{
	unsigned char *ptr = (unsigned char*)dest;
	while(count--)
		*ptr++ = (unsigned char)c;
	return dest;
}

size_t strlen(const char* str)
{
	const char *movingPtr=str;
	while (*movingPtr !=0 ) movingPtr++;
	return movingPtr-str;
}

void *memcpy(void *dest,const void *src,size_t count)
{
	char *destPtr=(char*)dest;
	const char *srcPtr=(const char*)src;
	while (count--)
		*destPtr++ = *srcPtr++;
	return dest;
}

int memcmp( const void *buf1, const void *buf2, size_t count)
{
	const char* p1=(const char*)buf1;
	const char* p2=(const char*)buf2;
	while (count--)
	{
		if (*p1<*p2) return -1;
		else if (*p1>*p2) return 1;
		p1++;
		p2++;
	}
	return 0;
}

#endif

