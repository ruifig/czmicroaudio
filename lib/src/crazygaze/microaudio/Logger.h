//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// Interface for logging and error reporting
//

#ifndef _CZ_LOGGER_H_
#define _CZ_LOGGER_H_

#include "Player.h"

namespace cz
{

/*!
* \brief Possible types of messages logged
*/
enum LogLevel
{
	LOG_NONE=0,
	LOG_ERROR,
	LOG_WARNING,
	LOG_INFO
};

/*!
* \brief Used to control and implement your own logging
*
* If you want logging, you should create a class that implements the required methods, and pass it
* in  \link Core::Create \endlink.
*/
class AudioLogger
{
public:
	/*!
	* \brief Callback for when an error occurs
	*
	* This is provided just for a quick way to detect errors in czPlayer during development,
	* when using a debug build.\n
	* For example, you can put an assert in your own implementation of this method. That
	* will allow you to make sure you catch any possible errors, and use the debugger to see where they happened (looking at the call stack).
	* 
	* \param errorCode
	*		The error code.
	*		You can use \link cz::GetErrorMsg \endlink to get a short description.
	* \warning
	*		You shouldn't rely on this in a release build, since that for performance and code size reasons, this callback may not be used.
	*
	*/
	virtual void OnError(int errorCode) = 0;

	/*!
	* \brief Callback for when logging needs to be done
	*
	* \param level You can use this to only log what you want (errors,warning,etc)
	* \param str The log message
	*/
	virtual void OnLog(LogLevel level, const char *str) = 0;
};

}

#endif
