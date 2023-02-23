//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// 
//

#include "czMemFile.h"
#include "czStandardC.h"


namespace cz
{
namespace io
{

MemFile::MemFile(::cz::Core *parentObject) : File(parentObject)
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

int MemFile::Open(u8 *ptr, int size, bool transferOwnership)
{
	m_memPtr = ptr;
	m_size = size;
	m_pos = 0;
	m_isOpen = true;
	m_ownsMemory = transferOwnership;
	return ERR_OK;	
}

int MemFile::Close(void)
{
	if (m_isOpen){
		if (m_ownsMemory)
		{
			CZFREE(m_memPtr);
		}
		m_memPtr = NULL;
		m_isOpen = false;
	}
	return ERR_OK;
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
	return ERR_OK;
}

int MemFile::ReadData(void *dest, int size)
{
//  :TODO: Disabled because Michael's doesn't know the size of resources
//	if (m_pos+size > m_size) CZERROR(ERR_IOERROR);
	memcpy(dest, &m_memPtr[m_pos], size);
	m_pos += size;	
	return ERR_OK;
}

int MemFile::WriteData(const void *src, int size)
{
	if (m_pos+size > m_size) CZERROR(ERR_IOERROR);
	memcpy(&m_memPtr[m_pos], src, size);
	m_pos +=size;	
	return ERR_OK;
}

} // namespace io
} // namespace cz
