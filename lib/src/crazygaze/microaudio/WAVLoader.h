//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
//
// --------------------------------------------------------------
//

#ifndef _CZ_WAVLOADER_H
#define _CZ_WAVLOADER_H

#include <crazygaze/microaudio/PlayerPrivateDefs.h>
#include <crazygaze/microaudio/Core.h>
#include <crazygaze/microaudio/StaticSound.h>
#include <crazygaze/microaudio/File.h>

namespace cz
{
namespace microaudio
{



class WAVDecoder;

class WAVLoader
{
public:
	WAVLoader(Core *core);
	~WAVLoader();

	int Load(::cz::io::File *in, StaticSound **basesnd);

	static bool CheckFormat(::cz::io::File *in);
private:
	Core *m_core;
	StaticSound *m_tmpSnd;
	WAVDecoder *m_decoder;

	void CleanupMemory();
};


} // end namespace microaudio
} // end namespace cz

#endif
