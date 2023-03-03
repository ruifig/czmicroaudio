//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// 
//

#pragma once

#include <stdio.h>
#include <crazygaze/microaudio/File.h>

#if CZMICROAUDIO_DISKFILE_ENABLED

namespace cz::microaudio
{

class DiskFile : public File 
{

public:
	DiskFile();
	virtual ~DiskFile();

	int Open(const char *filename, int resid, int mode);

	virtual int Close(void) override;
	virtual int GetPos(void) override;
	virtual int Seek(int pos,	FileSeekOrigin origin) override;
	virtual int ReadData(void *dest, int size) override;
	virtual int WriteData(const void *src, int size) override;

private:

#if CZ_PLATFORM_WIN32
	FILE* m_file;
#else
	// Throw an error to force me to revise this for new platforms
	#error Unknown or unsupported platform
#endif

};

} // namespace cz::microaudio


#endif // CZMICROAUDIO_DISKFILE_ENABLED


