//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// 
//

#include <crazygaze/microaudio/DiskFile.h>
#include <crazygaze/microaudio/PlayerPrivateDefs.h>


#if CZ_PLAYER_DISKFILE_ENABLED

namespace cz
{
namespace io
{

DiskFile::DiskFile(::cz::Core *parentObject) : File(parentObject)
{
}


DiskFile::~DiskFile()
{
	if (m_isOpen)
	{
		Close();
	}
}

int DiskFile::Open(const char *filename, int resid, int mode)
{
	//
	// Generic stdio
	//

	const char* openmode=NULL;
	if (mode==FILE_WRITE) openmode = "wb";
	else if (mode==FILE_UPDATE) openmode = "r+b";
	else openmode = "rb";

	if ((m_file = fopen(filename, openmode))==NULL) {
		// if open fails and the requested mode is UPDATE, we try to create the file (instead of open an existing one)
		if (mode==FILE_UPDATE){
			if ((m_file=fopen(filename,"w+b"))==NULL) CZERROR(ERR_CANTOPEN);
		} else {
			CZERROR(ERR_CANTOPEN);
		}
	}	

	m_isOpen = true;
	return ERR_OK;
}


int DiskFile::Close(void)
{
	CZASSERT(m_isOpen);

	if (fclose(m_file)!=0) CZERROR(ERR_CANTCLOSE);

	m_isOpen = false;
	return ERR_OK;
}



int DiskFile::GetPos(void)
{
	CZASSERT(m_isOpen);

	int pos=0;

	if ((pos=ftell(m_file))==-1) CZERROR(ERR_IOERROR);

	return pos;
}


int DiskFile::Seek(int pos , FileSeekOrigin origin)
{
	CZASSERT(m_isOpen);

	if (origin==::cz::io::FILE_SEEK_START)
	{
		if (fseek(m_file, pos, SEEK_SET)!=0) CZERROR(ERR_IOERROR);
	}
	else if (origin==FILE_SEEK_CURRENT)
	{
		if (fseek(m_file, pos, SEEK_CUR)!=0) CZERROR(ERR_IOERROR);
	}
	else
	{
		if (fseek(m_file, pos, SEEK_END)!=0) CZERROR(ERR_IOERROR);
	}

	return ERR_OK;
}

int DiskFile::ReadData(void *dest, int size)
{
	CZASSERT(m_isOpen);

	if (fread(dest,size,1,m_file)!=1) CZERROR(ERR_IOERROR);

	return ERR_OK;
}

int DiskFile::WriteData(const void *src, int size)
{
	CZASSERT(m_isOpen);

	if (size==0) return ERR_OK;
	if (fwrite(src,size,1,m_file)!=1){
		CZERROR(ERR_IOERROR);
	}
	fflush(m_file);

	return ERR_OK;
}


} // namesace io
} // namespace cz

#endif //

