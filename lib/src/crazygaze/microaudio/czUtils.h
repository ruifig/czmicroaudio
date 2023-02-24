//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// 
//
#ifndef _CZUTILS_H
#define _CZUTILS_H

namespace cz
{
namespace audio
{
	static uint16_t ByteSwap16(uint16_t n)
	{
		return (  ((((uint16_t) n) << 8) & 0xFF00) | \
			((((uint16_t) n) >> 8) & 0x00FF) );
	}


	static uint32_t ByteSwap32(uint32_t n)
	{
		return (  ((((uint32_t) n) << 24) & 0xFF000000) | \
			((((uint32_t) n) << 8) & 0x00FF0000)  | \
			((((uint32_t) n) >> 8) & 0x0000FF00)  | \
			((((uint32_t) n) >> 24) & 0x000000FF) );
	}

} // namespace audio
} // namespace cz

#endif
