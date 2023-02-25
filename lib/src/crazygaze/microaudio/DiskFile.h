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

#include <crazygaze/microaudio/File.h>

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

#if CZ_PLATFORM_WIN32
	FILE* m_file;
#else
	// Throw an error to force me to revise this for new platforms
	#error Unknown or unsupported platform
#endif

};


} // namespace io
} // namespace cz


#endif // CZ_PLAYER_DISKFILE_ENABLED

#endif
