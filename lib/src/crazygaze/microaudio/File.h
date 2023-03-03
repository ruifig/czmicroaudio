//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// Base class for IO
//

#pragma once

#include <crazygaze/microaudio/Core.h>

namespace cz::microaudio
{

enum FileOpenMode
{
	//! Opens for writing (its created if it doesn't exist)
	FILE_WRITE=0,
	//! Opens for updating (Read & Write)
	FILE_UPDATE=1,
	//! Opens for read.
	FILE_READ=2	
};

//! Seek types
/*!
* \sa \link czFile::Seek \endlink
*/
enum FileSeekOrigin
{
	FILE_SEEK_START, //!< Seek from start
	FILE_SEEK_CURRENT, //!< Seek from current position
	FILE_SEEK_END //!< Seek from end
};


//! Allows to read/write from files from different sources, with the same interface.
/*!
 * 
 * Usage is oriented for binary data.
 */
class File
{
	
public:

	//! Default constructor
	File();
	
	//! Destructor
	virtual ~File();

	//! Tells if a file is open
	/*!
	 * \return true if file is open, false if not
	 */
	bool IsOpen(void)
	{
		return m_isOpen;
	}
	
	//! Close the file
	/*!
	* \return Error::Success on success, other on error
	*/
	virtual int Close(void) = 0;


	//! Returns the position
	/*!
	 * \return The position if >0, or an error if <0
	 */
	virtual int GetPos(void) = 0;

	//! Change the position for the next read/write operation
	/*!
	 * Similar to C fseek function
	 * \return Error::Success on success, other on error
	 */
	virtual int Seek(int pos , FileSeekOrigin origin) = 0;
	
	//! Returns the size of the file
	virtual int GetSize(void);

	//! Reads data from the file.
	/*!
	 * \param dest Destination buffer
	 * \param size Number of bytes to read
	 * \return Error::Success on success, other on error
	 */	
	virtual int ReadData(void *dest, int size)  = 0;	

	//! Writes data to the file
	/*!
	 * \param src data to write.
	 * \param size number of bytes to write
	 * \return Error::Success on success, other on ERROR
	 */
	virtual int WriteData(const void *src, int size)  = 0;

	//!	Writes a string the file, followed by the newline characters
	/*!
	 * This is more like an utility function, with limited use.
	 * 
	 * Basically, it writes all the entire string to the file followed by the newline 
	 * character(s).
	 * 
	 * \param str String to write
	 * \return Error::Success on success, other on error
	 */
	int WriteString(const char *str);
	//void Log(const char *fmt, ...);
	
	//! Reads 1 Bytes from the file
	uint8_t Read8(void)
	{
		uint8_t dumb;
		ReadData(&dumb, sizeof(dumb));
		return dumb;
	}
	//! Reads 2 Bytes from the file
	uint16_t Read16(){
		uint16_t dumb;
		ReadData(&dumb, sizeof(dumb));
		if (m_swapBytes)
			dumb = BByteSwap16(dumb);
		return dumb;
	}
	//! Reads 4 Bytes from the file
	uint32_t Read32()
	{
		uint32_t dumb;
		ReadData(&dumb, sizeof(dumb));
		if (m_swapBytes)
			dumb = BByteSwap32(dumb);
		return dumb;
	}

	//! Reads an unsigned 8 bits integer from the file
	uint8_t ReadUnsigned8(void)
	{
		return Read8();
	}
	//! Reads an unsigned 16 bits integer from the file
	uint16_t ReadUnsigned16()
	{
		return Read16();
	}
	//! Reads an unsigned 32 bits integer from the file
	uint32_t ReadUnsigned32()
	{
		return Read32();
	}
	//! Reads an signed 8 bits integer from the file
	int8_t ReadSigned8(void)
	{
		return Read8();
	}
	//! Reads an signed 16 bits integer from the file
	int16_t ReadSigned16()
	{
		return Read16();
	}
	//! Reads an signed 32 bits integer from the file
	int32_t ReadSigned32()
	{
		return Read32();
	}
	
	//! Sets On/Off the automatic byte swapping when reading 16/32 bits numbers
	void SetSwapBytes(bool state)
	{
		m_swapBytes = state;
	}
	bool GetSwapBytes()
	{
		return m_swapBytes;
	}

protected:
	bool m_swapBytes;
	bool m_isOpen;
	bool m_isInsideWriteString; // used to avoid re-entrance into the ::Log
	
protected:

	static uint16_t BByteSwap16(uint16_t n)
	{
		return (  ((((uint16_t) n) << 8) & 0xFF00) | 
				  ((((uint16_t) n) >> 8) & 0x00FF) );
	}


	static uint32_t BByteSwap32(uint32_t n)
	{
		return (  ((((uint32_t) n) << 24) & 0xFF000000) |
				  ((((uint32_t) n) << 8) & 0x00FF0000)  |
				  ((((uint32_t) n) >> 8) & 0x0000FF00)  |
				  ((((uint32_t) n) >> 24) & 0x000000FF) );
	}
	
};

} // namespace cz::microaudio

