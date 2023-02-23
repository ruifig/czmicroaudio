/********************************************************************
	CrazyGaze (http://www.crazygaze.com)
	Author : Rui Figueira
	Email  : rui@crazygaze.com
	
	purpose:
	
*********************************************************************/

#ifndef _czPlayer_h_
#define _czPlayer_h_

#include "czPlatformStableHeaders.h"
#include <stddef.h>
#include <Mmsystem.h>

#ifndef NULL
#define	NULL	0L
#endif

#include "czPlayerConfig.h"

namespace cz
{

	/*!
	 * Possible error codes returned by methods.\n
	 * Check the \link AudioLogger \endlink class to help out with error detection.
	 */
	enum
	{
		ERR_OK=0, //! Indicates no error
		ERR_NOMEM=-1, //! Out of memory error
		//! An invalid parameter was passed
		ERR_INVPAR=-2,
		//! Resource,hardware or option not available or not detected
		/*!
		Returned when an option,resource, capability, hardware, etc is not 
		available or was not detected.
		*/
		ERR_NOTAVAILABLE=-3 ,
		//! A function was called prior to other.
		/*!
		This error is returned when you call a function that can't execute.

		e.g. If you call a member function of an object that 
		needs some initialization,or is in an invalid state.
		Other situations are when you try to initialize something that is 
		already initialized.
		*/
		ERR_CANTRUN=-4,
		//! Error opening a file
		/*!
		This error appears when an attempt to open/create a file failed.
		*/
		ERR_CANTOPEN=-5,
		//! Error closing file
		/*!
		This error indicates that a file couldn't be closed.
		*/
		ERR_CANTCLOSE=-6,
		//! Error writing/reading to/from a file
		/*!
		An error occured in a read/write operation with file.
		*/
		ERR_IOERROR=-7,
		//! Invalid file format
		/*!
		Means you tried to open a file whose format is invalid, or requires some features
		not supported at the moment.
		*/
		ERR_WRONGFORMAT=-8,
		//! Invalid data readed from file
		/*!
		Occurs when data readed from a file doesn't match the kind of data expected.
		*/
		ERR_INVALIDDATA=-9,
		//! Call to underlying API failed.
		/*!
		* This is returned when a call to the underlying system API fails.
		*/
		ERR_BADAPICALL=-10,
		//! An unknown error occurred
		ERR_UNKNOWN=-11

	};
} // namespace cz

#endif // _czPlayer_h_
