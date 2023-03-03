//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// 
//

#include <crazygaze/microaudio/MemFile.h>
#include <crazygaze/microaudio/PlayerPrivateDefs.h>
#include <crazygaze/microaudio/Memory.h>

namespace cz::microaudio
{

MemFile::MemFile()
{	
	m_pos = 0;
	m_memPtr = NULL;
	m_size = 0;
	m_ownsMemory = false;
}

MemFile::~MemFile()
{
	Close();	
}

int MemFile::Open(uint8_t *ptr, int size, bool transferOwnership)
{
	m_memPtr = ptr;
	m_size = size;
	m_pos = 0;
	m_isOpen = true;
	m_ownsMemory = transferOwnership;
	return Error::Success;	
}

int MemFile::Close(void)
{
	if (m_isOpen){
		if (m_ownsMemory)
		{
			CZMICROAUDIO_FREE(m_memPtr);
		}
		m_memPtr = NULL;
		m_isOpen = false;
	}
	return Error::Success;
}

int MemFile::GetPos(void)
{
	return m_pos;	
}

int MemFile::Seek(int pos, FileSeekOrigin origin)
{
	if (origin==FILE_SEEK_START)
		m_pos = pos;
	else if (origin==FILE_SEEK_CURRENT)
		m_pos += pos;
	else
		m_pos = m_size + pos;
	return Error::Success;
}

int MemFile::ReadData(void *dest, int size)
{
	if (m_pos+size > m_size) CZERROR(Error::IOError);
	memcpy(dest, &m_memPtr[m_pos], size);
	m_pos += size;	
	return Error::Success;
}

int MemFile::WriteData(const void *src, int size)
{
	if (m_pos+size > m_size) CZERROR(Error::IOError);
	memcpy(&m_memPtr[m_pos], src, size);
	m_pos +=size;	
	return Error::Success;
}

} // namespace cz::microaudio

