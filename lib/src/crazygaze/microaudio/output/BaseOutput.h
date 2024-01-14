#pragma once

#include <crazygaze/microaudio/Defs.h>

namespace cz::microaudio
{

// forward declarations
class AudioSource;

/*
* Sound output format
*
* This specifies the desired output format. The actual output device used might end up using different settings if the
* requested settings are not supported.
*/
struct OutputConfig
{
	/*!
	* \brief Frequency in Hertz.
	*
	* Ex: 11025, 22050, 44100.
	*
	* \note Specify 0 to use the recommended value.
	* \note Supported values depend on the driver type used.
	*/
	uint32_t sampleRate = 0;

	/*!
	* \brief 8 for 8 bits output, or 16 for 16 bits output
	* Valid values: 8, 16
	*
	* \note Specify 0 to use recommended value.
	* \note Supported values depend on the driver type used.
	*/
	uint8_t bitDepth = 0;

	/*!
	* \brief Number of output channels.
	* This is NOT the number of channels for mixing. Its to specify mono or stereo
	*
	* Valid values : 1 for mono, 2 for stereo
	* \note Specify 0 to use recommended value.
	* \note Supported values depend on the driver type used
	*/
	uint8_t numChannels = 0;

	/*!
	* \brief Size of the buffer to use, in milliseconds.
	*
	* Lower values decrease latency. Higher values increase stability.
	*
	* \note Specify 0 to use the recommended value.
	*/
	uint16_t bufSizeMs = 0;

	/*
	* Platform specific options.
	* If this is specified, then it should be a pointer to struct specific to the output being used.
	* E.g: If on Arduino, using the ArduinoI2SOutput, this should be a pointer to a ArduinoI2SOutput::Options struct
	*/
	void* platformOptions = nullptr;
};

/*!
* This class provides the interface to make things happens. Output sound.
*/
class BaseOutput
{
public:	
	BaseOutput() {}
	virtual ~BaseOutput() {}
	virtual Error begin(AudioSource& source, const OutputConfig* cfg = nullptr) = 0;
	virtual Error stop() = 0;
	virtual uint32_t getSampleRate() const = 0;
	virtual uint8_t getBitDepth() const = 0;
	virtual uint8_t getNumChannels() const = 0;
};

} // namespace cz::microaudio

