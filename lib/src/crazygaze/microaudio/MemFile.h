//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// This class simulates a file in memory, with a given memory buffer.
// 

#pragma once

#include <crazygaze/microaudio/File.h>

namespace cz::microaudio
{

class MemFile : public File
{

public:
	MemFile();

	virtual ~MemFile();
	
	// \param transferOwnership
	//		If this is true, then the object will own the memory block, and will deallocate it when destroyed
	int Open(uint8_t *ptr, int size, bool transferOwnership=false);
	virtual int Close(void) override;
	virtual int GetPos(void) override;
	virtual int Seek(int pos, FileSeekOrigin origin) override;
	virtual int ReadData(void *dest, int size) override;
	virtual int WriteData(const void *src, int size) override;

private:
	int m_pos;
	uint8_t *m_memPtr;
	int m_size;
	bool m_ownsMemory;
};

} // namespace cz::microaudio

