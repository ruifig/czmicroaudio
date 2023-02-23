//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
//
// --------------------------------------------------------------
//

#ifndef _CZ_WAVLOADER_H
#define _CZ_WAVLOADER_H

#include "czPlayerPrivateDefs.h"

#include "czCore.h"
#include "czStaticSound.h"
#include "czFile.h"

namespace cz
{
namespace audio
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


} // end namespace audio
} // end namespace cz

#endif
