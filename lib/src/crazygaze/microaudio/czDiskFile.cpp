//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// 
//

#include "czDiskFile.h"
#include "czStandardC.h"


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

#if CZ_PLATFORM==CZ_PLATFORM_SYMBIAN

	//
	// Symbian
	//

	TFileName name;
	name.Copy(TPtrC8((u8*)filename));
	CompleteWithAppPath(name);
#if CZ_PLATFORM_SIMULATOR
	name[ 0 ] = 'C';
#endif


	CZASSERT(!m_isOpen);
	m_fs.Connect();
	int m=0;
	if (mode == FILE_READ) { m = EFileRead | EFileStream; }
	else if (mode == FILE_UPDATE) { m = EFileWrite | EFileStream; }
	else if (mode == FILE_WRITE) { m = EFileWrite | EFileStream; }
	else CZERROR(ERR_INVPAR);

	int ret = m_file.Open(m_fs, name, m);
	// If the file does not exists, and the mode is WRITE, then create the file
	if ((ret == KErrNotFound)&& (mode == FILE_WRITE)){
		ret = m_file.Create(m_fs, name, EFileWrite | EFileStream);
	}
	if (ret != KErrNone) CZERROR(ERR_CANTOPEN);

	if (mode == FILE_WRITE) {
		m_file.SetSize(0);
	}

	if (ret<0){
		m_fs.Close();
		CZERROR(ERR_CANTOPEN);
	}

#else

	//
	// Generic stdio
	//

	char* openmode=NULL;
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


#endif

	m_isOpen = true;
	return ERR_OK;
}


int DiskFile::Close(void)
{
	CZASSERT(m_isOpen);

#if CZ_PLATFORM==CZ_PLATFORM_SYMBIAN
	m_file.Close();
	m_fs.Close();
#else
	if (fclose(m_file)!=0) CZERROR(ERR_CANTCLOSE);
#endif

	m_isOpen = false;
	return ERR_OK;
}



int DiskFile::GetPos(void)
{
	CZASSERT(m_isOpen);

	int pos=0;

#if CZ_PLATFORM==CZ_PLATFORM_SYMBIAN	
	int ret = m_file.Seek(ESeekCurrent, pos);
	if (ret!=KErrNone) CZERROR(ERR_IOERROR);
#else
	if ((pos=ftell(m_file))==-1) CZERROR(ERR_IOERROR);
#endif

	return pos;
}


int DiskFile::Seek(int pos , FileSeekOrigin origin)
{
	CZASSERT(m_isOpen);

#if CZ_PLATFORM==CZ_PLATFORM_SYMBIAN
	if (origin==::cz::io::FILE_SEEK_START)
	{
		if (m_file.Seek(ESeekStart, pos)!=KErrNone) CZERROR(ERR_IOERROR);
	}
	else if (origin==FILE_SEEK_CURRENT)
	{
		if (m_file.Seek(ESeekCurrent, pos)!=KErrNone) CZERROR(ERR_IOERROR);
	}
	else
	{
		if (m_file.Seek(ESeekEnd, pos)!=KErrNone) CZERROR(ERR_IOERROR);
	}

#else

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

#endif

	return ERR_OK;
}

int DiskFile::ReadData(void *dest, int size)
{
	CZASSERT(m_isOpen);

#if CZ_PLATFORM==CZ_PLATFORM_SYMBIAN	
	TPtr8 ptr((u8*)dest, size);
	if (m_file.Read(ptr)!=KErrNone) CZERROR(ERR_IOERROR);
#else
	if (fread(dest,size,1,m_file)!=1) CZERROR(ERR_IOERROR);
#endif

	return ERR_OK;
}

int DiskFile::WriteData(const void *src, int size)
{
	CZASSERT(m_isOpen);

	if (size==0) return ERR_OK;
#if CZ_PLATFORM==CZ_PLATFORM_SYMBIAN	
	TPtrC8 ptr((u8*)src, size);
	if (m_file.Write(ptr)!=KErrNone) CZERROR(ERR_IOERROR);
#else
	if (fwrite(src,size,1,m_file)!=1){
		CZERROR(ERR_IOERROR);
	}
	fflush(m_file);
#endif

	return ERR_OK;
}


} // namesace io
} // namespace cz

#endif //
