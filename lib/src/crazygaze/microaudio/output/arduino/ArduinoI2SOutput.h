#include <crazygaze/microaudio/output/BaseOutput.h>
#include "I2S.h"

#pragma once

namespace cz::microaudio
{

class ArduinoI2SOutput : public BaseOutput
{
public:
	struct Options
	{
		/*
		* What pin to use for I2S BCLK.
		* The LRCLK will be BCLKPin+1
		* See https://arduino-pico.readthedocs.io/en/latest/i2s.html
		*/
		int BCLKPin = 26;

		/*
		* DATA output pin
		* See https://arduino-pico.readthedocs.io/en/latest/i2s.html
		*/
		int DATAPin = 28;
	};

	ArduinoI2SOutput();
	virtual ~ArduinoI2SOutput();
	virtual Error begin(AudioSource& source, const OutputConfig* cfg) override;
	virtual Error stop() override;
	virtual uint32_t getSampleRate() const override;
	virtual uint8_t getBitDepth() const override;
	virtual uint8_t getNumChannels() const override;
private:

	void onTransmit();
	static void onTransmitCallback();
	I2S m_i2s;
	Options m_cfgOptions;
	OutputConfig m_cfg;
	AudioSource* m_source = nullptr;
	inline static ArduinoI2SOutput* ms_instance = nullptr;
};

} // cz::microaudio
