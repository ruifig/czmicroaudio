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
		* DATA output pin
		* See https://arduino-pico.readthedocs.io/en/latest/i2s.html
		*/
		int DATAPin = 22;

		/*
		* What base pin to use for BCLK and LRCLK.
		* 
		* This defines what pins to use for BCLK and LRCLK, since they need to be consecutive.
		* Which one is first depends on the "swapClocks" field.
		*
		* See https://arduino-pico.readthedocs.io/en/latest/i2s.html
		*/
		int baseClockPin = 27;

		/*
		* Arduino-pico core (see https://arduino-pico.readthedocs.io/en/latest/i2s.html) expects the BCLK/LRCLK pin pair as BCLK first,
		* but some I2S boards (e.g: Rev2.1 of https://www.waveshare.com/wiki/Pico-Audio ), expect LRCLK first.
		* Settign this to true swaps of those pins, thus supporting such boards.
		*/
		bool swapClocks = false;
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
