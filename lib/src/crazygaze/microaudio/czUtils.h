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
	static u16 ByteSwap16(u16 n)
	{
		return (  ((((u16) n) << 8) & 0xFF00) | \
			((((u16) n) >> 8) & 0x00FF) );
	}


	static u32 ByteSwap32(u32 n)
	{
		return (  ((((u32) n) << 24) & 0xFF000000) | \
			((((u32) n) << 8) & 0x00FF0000)  | \
			((((u32) n) >> 8) & 0x0000FF00)  | \
			((((u32) n) >> 24) & 0x000000FF) );
	}

} // namespace audio
} // namespace cz

#endif
