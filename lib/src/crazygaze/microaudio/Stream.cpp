//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// 
//

#include <crazygaze/microaudio/Stream.h>
#include <crazygaze/microaudio/DiskFile.h>

namespace cz::microaudio
{

#if CZMICROAUDIO_OGG_ENABLED
// Ogg callbacks to read from our stream
size_t OggRead (void *ptr, size_t size, size_t nmemb, void *datasource)
{
	File* f = static_cast<File*>(datasource);
	size_t dataLeft = f->GetSize() - f->GetPos();
	size_t nmemb_left = dataLeft/size;
	int nnemb_toread = MIN(nmemb_left, nmemb);
	if (f->ReadData(ptr, size*nnemb_toread)==Error::Success)
		return nnemb_toread;
	else
		return 0;
}

int OggSeek (void *datasource, ogg_int64_t offset, int whence)
{
	File* f = static_cast<File*>(datasource);
	int res = 0;
	if (whence==SEEK_SET)
		res = f->Seek(static_cast<int>(offset), FILE_SEEK_START);
	else if (whence==SEEK_CUR)
		res = f->Seek(static_cast<int>(offset), FILE_SEEK_CURRENT);
	else if (whence==SEEK_END)
		res = f->Seek(static_cast<int>(offset), FILE_SEEK_END);
	else
	{
		assert(0);
	}

	return (res==Error::Success) ? 0 : -1;
}

int OggClose (void *datasource)
{
	File* f = static_cast<File*>(datasource);
	int res = f->Close();
	return (res==Error::Success) ? 0 : -1;
}

long OggTell (void *datasource)
{
	File* f = static_cast<File*>(datasource);
	return f->GetPos();
}
#endif

//////////////////////////////////////////////////////////////////////////
// StreamSound
//////////////////////////////////////////////////////////////////////////

StreamSound::StreamSound()
{
	m_isplaying = false;
	m_lastPos = 0;
	m_loop = false;
	m_framesToMixBeforeFinish = -1;
}

StreamSound::~StreamSound()
{
}

int StreamSound::Init(UniquePtr<File> in, int workBufferNumFrames)
{
	m_in = std::move(in);

#if CZMICROAUDIO_OGG_ENABLED
	ov_callbacks callbacks;
	callbacks.read_func = OggRead;
	callbacks.seek_func = OggSeek;
	callbacks.close_func = OggClose;
	callbacks.tell_func = OggTell;

	int ret=0;
	ret = ov_open_callbacks(m_in, &m_ogg, NULL, 0, callbacks);
	if (ret)
	{
		CZERROR(Error::BadAPICall);
	}

	// Streams needs to be seekable so we can loop/restart
	if (ov_seekable(&m_ogg)==0)
		CZERROR(Error::WrongFormat);

	vorbis_info* info = ov_info(&m_ogg, -1);
	if (!info)
		CZERROR(Error::BadAPICall);


	if (!(info->channels==1 || info->channels==2))
		CZERROR(Error::WrongFormat);

	m_snd.Set(SOUND_16BITS|SOUND_SIGNED| ((info->channels==1) ? SOUND_MONO : SOUND_STEREO), workBufferNumFrames);
	m_snd.SetDefaults(info->rate);
#else
	m_snd.Set(SOUND_16BITS|SOUND_SIGNED|SOUND_MONO, workBufferNumFrames);
	m_snd.SetDefaults(11025);
#endif

	return Error::Success;
}

int StreamSound::PrepareToPlay(bool loop)
{
	if (m_isplaying)
	{
		CZERROR(Error::CantRun);
	}

	m_lastPos = 0;
	m_framesToMixBeforeFinish = -1;
	m_loop = loop;
#if CZMICROAUDIO_OGG_ENABLED
	ov_time_seek(&m_ogg, 0);
#endif
	m_snd.SetToSilence();
	m_isplaying = true;
	return Error::Success;
}

void StreamSound::FinishedPlaying()
{
	m_isplaying = false;
}

int StreamSound::Decode(void* dest, int numframes)
{
#if CZMICROAUDIO_OGG_ENABLED
	int current_section;
	long ret;
	int bytesTodo = numframes*m_snd.GetFrameSizeBytes();
	char* ptr = static_cast<char*>(dest);
	while (bytesTodo)
	{
		ret = ov_read(&m_ogg, ptr, bytesTodo, &current_section);
		if (ret>0) // No errors
		{
			bytesTodo -= ret;
			ptr += ret;
		}
		else if (ret==0) // EOF
		{
			if (m_loop)
				ov_time_seek(&m_ogg, 0);
			else
			{
				// Set remaining to silence
				memset(ptr, 0, bytesTodo);
				break;
			}
		}
		else // <0 means error
		{
			CZERROR(Error::BadAPICall);
			return -1;
		}
	}

	int framesDone = numframes - bytesTodo/m_snd.GetFrameSizeBytes();
	CZASSERT(framesDone<=numframes);

	/*
	static DiskFile out(m_core);
	if (!out.IsOpen())
	{
		out.Open("oggout.raw", 0, FILE_WRITE);
	}
	out.WriteData(dest, framesDone*m_snd.GetFrameSizeBytes());
	*/

	return framesDone;
#else
	return numframes;
#endif
}

bool StreamSound::ChannelMix(int mixpos, int numframes)
{

	// If we're finishing, then keep writing silence until we're done
	if (m_framesToMixBeforeFinish>=0)
	{
		m_framesToMixBeforeFinish -= numframes;
		if (mixpos>m_lastPos)
		{
			memset(m_snd.GetPtrToFrame(m_lastPos), 0, mixpos - m_lastPos);
		}
		else if (mixpos<m_lastPos)
		{
			memset(m_snd.GetPtrToFrame(m_lastPos), 0, m_snd.GetNumFrames()-m_lastPos);
			memset(m_snd.GetPtrToFrame(0), 0, mixpos);
		}

		m_lastPos = mixpos;
		return (m_framesToMixBeforeFinish>0) ? false : true;
	}

	int section=0;
	if (mixpos>m_lastPos)
	{
		int todo = mixpos-m_lastPos;
		int framesdone = Decode(m_snd.GetPtrToFrame(m_lastPos), todo);

		if (m_lastPos==0 && framesdone)
		{
			// We need to copy some frames to the end of the buffer, otherwise we'll get clicks when mixing at the end, since linear/cubic interpolation
			// will grab samples from outside
			memcpy(m_snd.GetPtrToFrame(m_snd.GetNumFrames()), m_snd.GetPtrToFrame(0), ::cz::microaudio::SOUND_SAFETYAREA*m_snd.GetFrameSizeBytes());
		}

		m_lastPos = mixpos;
		if (framesdone==-1) // Some error occured
		{
			return false;
		}
		else if (framesdone<todo) // stream finished and we don't want looping
		{
			m_framesToMixBeforeFinish = m_snd.GetNumFrames();
		}
	}
	else if (mixpos<m_lastPos)
	{
		int todo1 = m_snd.GetNumFrames()-m_lastPos;
		int todo2 = mixpos;
		int framesDone1 = Decode(m_snd.GetPtrToFrame(m_lastPos), todo1);
		int framesDone2 = Decode(m_snd.GetPtrToFrame(0), todo2);

		// We need to copy some frames to the end of the buffer, otherwise we'll get clicks when mixing at the end, since linear/cubic interpolation
		// will grab samples from outside
		if (framesDone2)
			memcpy(m_snd.GetPtrToFrame(m_snd.GetNumFrames()), m_snd.GetPtrToFrame(0), ::cz::microaudio::SOUND_SAFETYAREA*m_snd.GetFrameSizeBytes());

		m_lastPos = mixpos;

		// Stream finished and we don't want looping, or some error occured
		if (framesDone1<todo1 || framesDone2<todo2)
		{
			m_framesToMixBeforeFinish = m_snd.GetNumFrames();
		}
	}
	else
	{
		m_lastPos = mixpos;
	}

	return true;
}

} // namespace cz::microaudio

