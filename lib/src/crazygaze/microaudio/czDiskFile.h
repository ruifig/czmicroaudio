//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// 
//

#ifndef _CZDISKFILE_H_
#define _CZDISKFILE_H_

#include <stdio.h>

#include "czPlayerPrivateDefs.h"
#include "czFile.h"

#if CZ_PLAYER_DISKFILE_ENABLED

namespace cz
{
namespace io
{

class DiskFile : public File 
{

public:
	DiskFile(::cz::Core *parentObject);
	virtual ~DiskFile();

	int Open(const char *filename, int resid, int mode);

	virtual int Close(void);
	virtual int GetPos(void);
	virtual int Seek(int pos,	FileSeekOrigin origin);
	virtual int ReadData(void *dest, int size);
	virtual int WriteData(const void *src, int size);

private:

#if CZ_PLATFORM==CZ_PLATFORM_SYMBIAN
	RFs m_fs;
	RFile m_file;
#else
	FILE* m_file;
#endif

};


} // namespace io
} // namespace cz


#endif // CZ_PLAYER_DISKFILE_ENABLED

#endif
