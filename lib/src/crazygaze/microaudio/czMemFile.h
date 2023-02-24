//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// This class simulates a file in memory, with a given memory buffer.
// 

#ifndef _CZMEMFILE_H_
#define _CZMEMFILE_H_

#include "czPlayerPrivateDefs.h"
#include "czFile.h"

namespace cz
{
namespace io
{

class MemFile : public File
{

public:
	MemFile(::cz::Core *parentObject);
	virtual ~MemFile();
	
	// \param transferOwnership
	//		If this is true, then the object will own the memory block, and will deallocate it when destroyed
	int Open(uint8_t *ptr, int size, bool transferOwnership=false);
	virtual int Close(void);
	virtual int GetPos(void);
	virtual int Seek(int pos, FileSeekOrigin origin);
	virtual int ReadData(void *dest, int size);
	virtual int WriteData(const void *src, int size);

private:
	int m_pos;
	uint8_t *m_memPtr;
	int m_size;
	bool m_ownsMemory;
};

} // namespace io
} // namespace cz


#endif
