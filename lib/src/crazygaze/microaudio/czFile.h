//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// Base class for IO
//


#ifndef _CZFILE_H_
#define _CZFILE_H_

#include "czPlayerPrivateDefs.h"
#include "czObject.h"

namespace cz
{
namespace io
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
class File : public ::cz::Object
{
	
public:

	//! Default constructor
	File(::cz::Core *core);
	
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
	* \return ERR_OK on success, other on error
	*/
	virtual int Close(void) = 0;


	//! Returns the position
	/*!
	 * \return The position if >0, or an error if <0
	 */
	virtual int GetPos(void) = 0;

	//! Change the position for the next read/write operation
	/*!
	 * Similiar to C fseek function
	 * \return ERR_OK on sucess, other on error
	 */
	virtual int Seek(int pos , FileSeekOrigin origin) = 0;
	
	//! Returns the size of the file
	virtual int GetSize(void);

	//! Reads data from the file.
	/*!
	 * \param dest Destination buffer
	 * \param size Number of bytes to read
	 * \return ERR_OK on success, other on error
	 */	
	virtual int ReadData(void *dest, int size)  = 0;	

	//! Writes data to the file
	/*!
	 * \param src data to write.
	 * \param size number of bytes to write
	 * \return ERR_OK on success, other on ERROR
	 */
	virtual int WriteData(const void *src, int size)  = 0;

	//!	Writes a string the file, followed by the newline characters
	/*!
	 * This is more like an utility function, with limited use.
	 * 
	 * Basically, it writes all the string to the file, followed by the newline 
	 * characters.
	 * 
	 * \param str String to write
	 * \return ERR_OK on success, other on error
	 */
	int WriteString(const char *str);
	//void Log(const char *fmt, ...);
	
	//! Reads 1 Bytes from the file
	u8 Read8(void)
	{
		u8 dumb;
		ReadData(&dumb, sizeof(dumb));
		return dumb;
	}
	//! Reads 2 Bytes from the file
	u16 Read16(){
		u16 dumb;
		ReadData(&dumb, sizeof(dumb));
		if (m_swapBytes)
			dumb = BByteSwap16(dumb);
		return dumb;
	}
	//! Reads 4 Bytes from the file
	u32 Read32()
	{
		u32 dumb;
		ReadData(&dumb, sizeof(dumb));
		if (m_swapBytes)
			dumb = BByteSwap32(dumb);
		return dumb;
	}

	//! Reads an unsigned 8 bits integer from the file
	u8 ReadUnsigned8(void)
	{
		return Read8();
	}
	//! Reads an unsigned 16 bits integer from the file
	u16 ReadUnsigned16()
	{
		return Read16();
	}
	//! Reads an unsigned 32 bits integer from the file
	u32 ReadUnsigned32()
	{
		return Read32();
	}
	//! Reads an signed 8 bits integer from the file
	s8 ReadSigned8(void)
	{
		return Read8();
	}
	//! Reads an signed 16 bits integer from the file
	s16 ReadSigned16()
	{
		return Read16();
	}
	//! Reads an signed 32 bits integer from the file
	s32 ReadSigned32()
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

	static u16 BByteSwap16(u16 n)
	{
		return (  ((((u16) n) << 8) & 0xFF00) | 
				  ((((u16) n) >> 8) & 0x00FF) );
	}


	static u32 BByteSwap32(u32 n)
	{
		return (  ((((u32) n) << 24) & 0xFF000000) |
				  ((((u32) n) << 8) & 0x00FF0000)  |
				  ((((u32) n) >> 8) & 0x0000FF00)  |
				  ((((u32) n) >> 24) & 0x000000FF) );
	}
	
	
};


} // namespace io
} // namespace cz

#endif
