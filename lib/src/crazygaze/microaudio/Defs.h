#pragma once

#include <crazygaze/microaudio/Config.h>

namespace cz::microaudio
{
	/*!
	 * Possible error codes returned by methods.\n
	 */
	enum class Error
	{
		//! Indicates no error
		Success=0,

		//! Out of memory error
		OutOfMemory=1, 

		//! An invalid parameter was passed
		InvalidParameter=2,

		//! Resource,hardware or option not available or not detected
		/*!
		Returned when an option,resource, capability, hardware, etc is not 
		available or was not detected.
		*/
		NotAvailable=3 ,

		//! A function was called prior to other.
		/*!
		This error is returned when you call a function that can't execute.

		e.g. If you call a member function of an object that 
		needs some initialization,or is in an invalid state.
		Other situations are when you try to initialize something that is 
		already initialized.
		*/
		CantRun=4,

		//! Error opening a file
		/*!
		This error appears when an attempt to open/create a file failed.
		*/
		CantOpen=5,

		//! Error closing file
		/*!
		This error indicates that a file couldn't be closed.
		*/
		CantClose=6,

		//! Error writing/reading to/from a file
		/*!
		An error occurred in a read/write operation with file.
		*/
		IOError=7,

		//! Invalid file format
		/*!
		Means you tried to open a file whose format is invalid, or requires some features
		not supported at the moment.
		*/
		WrongFormat=8,

		//! Invalid data read from file
		/*!
		Occurs when data read from a file doesn't match the kind of data expected.
		*/
		InvalidData=9,

		//! Call to underlying API failed.
		/*!
		* This is returned when a call to the underlying system API fails.
		*/
		BadAPICall=10,

		//! An unknown error occurred
		Unknown=11
	};

	enum class LogLevel
	{
		None,
		Fatal,
		Error,
		Warning,
		Log
	};

	enum class InterpolationMode
	{
		/*! No interpolation.
		* This method has the best performance but lacks the quality.
		*/
		None,

		/*! Linear interpolation
		* Overall good interpolation mode. Not a lot slower than no interpolation and wit and with reasonable quality.
		*/
		Linear
	};

	/*
	* 0 - Full panning to the left
	* 128 - Center panning
	* 255 - Full panning to the right
	*/
	using PanValue = uint8_t;

	/*
	* Full range of volume is 0-255
	*/
	using Volume = uint8_t;


}

