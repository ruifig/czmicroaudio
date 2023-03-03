//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
//
// --------------------------------------------------------------
//

#pragma once

#include <crazygaze/microaudio/PlayerPrivateDefs.h>
#include <crazygaze/microaudio/Core.h>
#include <crazygaze/microaudio/StaticSound.h>
#include <crazygaze/microaudio/File.h>

namespace cz::microaudio
{
class WAVDecoder;

class WAVLoader
{
public:
	WAVLoader();
	~WAVLoader();

	int Load(File *in, StaticSound **basesnd);

	static bool CheckFormat(File *in);
private:
	StaticSound *m_tmpSnd;
	WAVDecoder *m_decoder;

	void CleanupMemory();
};

} // end namespace cz::microaudio

