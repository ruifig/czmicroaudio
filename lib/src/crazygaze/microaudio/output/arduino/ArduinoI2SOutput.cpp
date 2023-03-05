#include <crazygaze/microaudio/output/arduino/ArduinoI2SOutput.h>
#include <crazygaze/microaudio/PlayerPrivateDefs.h>
#include <crazygaze/microaudio/AudioSource.h>

namespace cz::microaudio
{

ArduinoI2SOutput::ArduinoI2SOutput()
	: m_i2s(OUTPUT)
{
	assert(ms_instance == nullptr);
	ms_instance = this;
}

ArduinoI2SOutput::~ArduinoI2SOutput()
{
	ms_instance = nullptr;
}

uint32_t ArduinoI2SOutput::getSampleRate() const
{
	return m_cfg.sampleRate;
}
uint8_t ArduinoI2SOutput::getBitDepth() const
{
	return m_cfg.bitDepth;
}

uint8_t ArduinoI2SOutput::getNumChannels() const
{
	return m_cfg.numChannels;
}

Error ArduinoI2SOutput::begin(AudioSource& source, const OutputConfig* cfg)
{
	m_source = &source;

	m_cfg.sampleRate = (cfg && cfg->sampleRate != 0) ? cfg->sampleRate : 48000;
	m_cfg.bitDepth = (cfg && cfg->bitDepth != 0 && (cfg->bitDepth==8 || cfg->bitDepth==16)) ? cfg->bitDepth : 16;
	m_cfg.numChannels = (cfg && cfg->numChannels != 0 && (cfg->numChannels==1 || cfg->numChannels==2)) ? cfg->numChannels : 2;
	m_cfg.bufSizeMs = (cfg && cfg->bufSizeMs != 0) ? cfg->bufSizeMs : 100;
	m_cfg.platformOptions = nullptr;
	if (cfg && cfg->platformOptions)
	{
		m_cfgOptions = *reinterpret_cast<const Options*>(cfg->platformOptions);
	}

	m_i2s.setBCLK(m_cfgOptions.BCLKPin);
	m_i2s.setDATA(m_cfgOptions.DATAPin);
	m_i2s.setBitsPerSample(m_cfg.bitDepth);
	m_i2s.setFrequency(m_cfg.sampleRate);

	m_i2s.onTransmit(onTransmitCallback);

	m_i2s.begin();

	return Error::Success;
}

Error ArduinoI2SOutput::stop()
{
	m_i2s.end();
	return Error::Success;
}

void ArduinoI2SOutput::onTransmitCallback()
{
	assert(ms_instance);
	ms_instance->onTransmit();
}

void ArduinoI2SOutput::onTransmit()
{
	constexpr int numFrames = 64;
	int16_t buffer[numFrames*2];

	int todo = m_i2s.availableForWrite();
	while(todo)
	{
		int portion = MIN(todo, numFrames);
		todo -= portion;
		m_source->get(buffer, portion);
		const int16_t* p = buffer;
		while(portion--)
		{
			m_i2s.write16(p[0], p[1]);
			p +=2;
		}
	}
}

} // cz::microaudio
