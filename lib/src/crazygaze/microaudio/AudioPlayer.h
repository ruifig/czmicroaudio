//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
//
// --------------------------------------------------------------
// 

#pragma once

#include "Player.h"
#include "Audio.h"
#include "AudioPlayerListener.h"

namespace cz::microaudio
{

/*!
 * \brief Configuration information for the %audio output device
 * 
 * The default constructor initializes all members to default values, so you don't need (or forget)
 * to set them all. You only need to specify the parameters you really want to change.
 *
 * \see  \link AudioPlayer::GetDefaultConfig \endlink
 */
struct AudioPlayerConfig
{
	/*!
	 * \brief Simply initialized all values to automatic
	 */
	AudioPlayerConfig()
		: driverType(AUDIO_DRIVER_DEFAULT), bufSizeMs(0), frequency(0),bits(0),outputChannels(0),
		maxChannels(0),interpolationMode(AUDIO_INTERPOLATION_NONE) {}

	/*!
	* \brief  Driver to type use.
	* \note Use #AUDIO_DRIVER_DEFAULT to use the default one.
	*/
	AudioDriverType driverType;

	/*!
	* \brief Size of the buffer to use, in milliseconds.
	*
	* Lower values decrease latency. Higher values increase stability.
	*
	* \note Specify 0 to use the recommended value.
	*/
	int bufSizeMs;

	/*!
	* \brief Frequency in Hertz.
	*
	* Ex: 11025, 22050, 44100.
	*
	* \note Specify 0 to use the recommended value.
	* \note Supported values depend on the driver type used.
	*/
	int frequency;

	/*!
	* \brief 8 for 8 bits output, or 16 for 16 bits output
	* Valid values: 8, 16
	*
	* \note Specify 0 to use recommended value.
	* \note Supported values depend on the driver type used.
	*/
	int bits;

	/*!
	* \brief Number of output channels.
	* This is NOT the number of channels for mixing. Its to specify mono,stereo, etc
	*
	* Valid values : 1 for mono, 2 for stereo
	* \note Specify 0 to use recommended value.
	* \note Supported values depend on the driver type used
	*/
	int outputChannels;

	/*!
	* \brief Maximum software channels to use for mixing
	*
	* Number of maximum channels to make available for mixing. Ex: 16, 32, etc.
	* Higher values mean you can have more sounds playing simultaneously, but it consumes more
	* CPU resources and memory.
	*
	* \note Specify 0 to use recommended value.
	*/
	int maxChannels;

	/*!
	* \brief Interpolation mode to use for software mixing
	*
	* \note Specify #AUDIO_INTERPOLATION_NONE to automatically select a good compromise.
	* 
	*/
	AudioInterpolationMode interpolationMode;
};


/*!
 * \brief Main class to play sounds.
 *
 * This is the class you'll use the most, to play sounds, stop, load, release, etc.
 * Use the static method #Create to create an instance, and #Destroy to destroy.
 */
class AudioPlayer
{
public:

	/*!
	 * \brief Provide a way to get the configuration used for the default driver type.
	 *
	 * Fills the passed structure with the configuration for the current platform's default output.\n
	 * You don't need to use this method. It's available just to let you know what is the default configuration used if you don't specify one.
	*/
	static int GetDefaultConfig(AudioPlayerConfig *cfg);

	/*!
	 * \brief Creates an audio player with the specified configuration.
	 *
	 * \param core
	 *		This is required to provide the context. See cz#Core for more information.
	 * \param cfg
	 *		Desired configuration. You can pass NULL to use the defaults or specify your own configuration.
	 * \sa \link Destroy \endlink
	 */
	static AudioPlayer* Create(AudioPlayerConfig *cfg=NULL);

	/*!
	 * \brief Destroys the object
	 *
	 * Note that \link AudioPlayer::Create \endlink is static, while this isn't.
	 */
	virtual void Destroy() = 0;

	/*!
	 * \brief Performs internal updates
	 *
	 * You should call this once in every tick of you game/application. Some 
	 * platforms don't require this to be called in order to work, but in the future
	 * it may be necessary.
	*/
	virtual int Update()=0;

#if CZMICROAUDIO_DISKFILE_ENABLED
	/*!
	 * \brief Loads a tracked song from an external file.
	 *
	 * \param filename The file to load
	 *
	 * Make sure if the specified file is a supported song format.\n
	 * Consult \ref Features to know the supported formats.
	 *
	 * \return The created object, or NULL on error
	 * \sa \link PlayModule \endlink, \link FreeModuleSound \endlink
	 */
	virtual HMODULEDATA LoadModule(const char* filename) = 0;
#endif

	/*!
	 * \brief Loads a Module from a memory buffer, instead from a file
	 *
	 * You can use this method when you have the file data already in memory.
	 * For example, you might choose to have all data in the executable, instead of external files.
	 * \param data Pointer to the data
	 * \param dataSize Size of the data
	 * \return The created object, or NULL on error
	 *
	 * \sa \link LoadModule(const char*) \endlink for more information.
	 * \sa \link PlayModule \endlink
	*/
	virtual HMODULEDATA LoadModule(const void* data, int dataSize) = 0;

#if CZMICROAUDIO_DISKFILE_ENABLED
	/*!
	 * \brief Load a WAV sound from a file
	 *
	 * Use this method to load a WAV file.
	 * \return The created object or NULL on error
	 * \sa \link Play \endlink
	 */
	virtual HSOUNDDATA LoadWAV(const char* filename) = 0;
#endif


#if CZMICROAUDIO_OGG_ENABLED

#if CZMICROAUDIO_DISKFILE_ENABLED
	virtual HMODULEDATA LoadStream(const char* filename) = 0;
#endif

	virtual HMODULEDATA LoadStream(const void* data, int dataSize) = 0;
	virtual int FreeStream(HSOUNDDATA handle) = 0;
#endif

	/*!
	* \brief Load a WAV sound from a memory buffer
	*
	* Use this method to load a WAV from a buffer you already have in memory.
	* \param data Pointer to the data
	* \param dataSize Size of the data
	* \return The created object or NULL on error
	* \sa \link Play \endlink
	*/
	virtual HSOUNDDATA LoadWAV(const void *data, int dataSize) = 0;

	/*!
	 * \brief Free a loded song
	 *
	 * \return Error::Success if success, other if error.
	 * \warning Don't call this for a song that is currently playing
	 * \sa \link LoadModule \endlink
	 */
	virtual int FreeModuleSound(HMODULEDATA handle) = 0;

	/*!
	 * \brief Free a loded sound
	 *
	 * \return Error::Success if success, other if error.
	 * \warning Don't call this for a sound that is currently playing.
	 * \sa \link LoadWAV \endlink
	 */
	virtual int FreeStaticSound(HSOUNDDATA handle) = 0;


	/*!
	 * \brief Changes the software interpolation mode
	 *
	 * You can change the quality whenever you want, even while playing
	 */
	virtual void SetInterpolationMode(AudioInterpolationMode quality) = 0;

	/*!
	 * \brief Gets the current software interpolation
	 */
	virtual AudioInterpolationMode GetInterpolationMode() = 0;

	/*!
	 * \brief Enables/Disales volume ramping
	 * \param numFrames
	 *		Size of the volume ramping.  Pass 0 to disable. 
	 * \note
	 *		Volume ramping is ignored if quality is set to \link AUDIO_INTERPOLATION_NONE \link
	 */
	virtual void SetVolumeRamping(int numFrames=64) = 0;

	/*!
	 * \brief Returns the size of volume ramping (0 means no volume ramping)
	 */
	virtual int GetVolumeRamping() = 0;

	/*!
	* \brief Enables/Disables sample stop ramping
	* \param numFrames
	*		Size of stop ramping.  Pass 0 to disable.
	* \note
	*		Sample stop ramping is ignored if quality is set to \link AUDIO_INTERPOLATION_NONE \link
	*/
	virtual void SetStopRamping(int numFrames=64) = 0;

	/*!
	* \brief Returns the size of stop ramping (0 means no stop ramping)
	*/
	virtual int GetStopRamping() = 0;

	/*!
	 * \brief Gets the output frequency in Hertz.
	 */
	virtual int GetOutputFrequency(void) = 0;

	/*!
	 * \brief Gets the maximum number of channels
	 * The maximum number of channels represents the number of sounds you can
	 * play simultaneously.
	 *
	 * \return The number of channels used for mixing.
	 */
	virtual int GetMaxChannels(void) = 0;

	/*!
	 * \brief Gets an aproximated number of channels currently in use.
	 * \return The number channels playing.
	 */
	virtual int GetChannelsPlaying(void) = 0;

	/*! Returns the number of output channels (ex: 2=stereo, 1=mono)
	 */
	virtual int GetOutputChannels(void) = 0;

	/*! Returns the output bits (ex : 16,8)
	 */
	virtual int GetOutputBits(void) = 0;


	/*! Pause the sound output
	 * \param freeResources
	 *		Used in some platforms to free up as many resources as possible when pausing.
	 *		When #ResumeOutput is called, it will create the necessary resources again.
	 *		For example, in Symbian, if you pass \c true, the mixer thread will be destroyed, and recreated again
	 *		in the next call to  #ResumeOutput . In some occasions, that's the desired behavior, when focus is
	 *		lost to answer a call, for example.
	 * 
	 */
	virtual void PauseOutput(bool freeResources) = 0;

	//! Resumes sound output.
	// \sa PauseOutput
	virtual void ResumeOutput() = 0;


	/*!
	 * \brief Plays a sound.
	 *
	 * \param sndDataHandle Sound data to play
	 * \param vol Volume. -1 to use default volume for this sound
	 * \param pan Panning. -1 to use default panning for this sound
	 * \param loopMode SOUND_LOOP_OFF, SOUND_LOOP_NORMAL, or SOUND_LOOP_BIDI
	 * \param loopStart Position where the loop starts.
	 *		Note that the sound always start playing at position 0. This is the position it loops to.
	 * \param loopLength The length of the loop, in sound frames.
	 *		Specify 0 if you just want it to loop to the end of the sound.
	 * \note If you pass a loop mode (normal or bidi), and let both "loopStart", and "loopLength" with the default values, it will
	 *		loop the entire sound.
	 * \return Returns the handler to control the sound, or -1 on error
	 * \warning
	 * You should not use this function to play songs. Check \link AudioPlayer::PlayModule \endlink for that
	 * \sa \link Stop \endlink
	 */
	virtual HSOUND Play(HSOUNDDATA sndDataHandle, int vol=-1, int pan=-1, LoopMode loopMode= SOUND_LOOP_OFF, int loopStart=0, int loopLength=0) = 0;

	//virtual HSOUND PlaySound2(HSOUNDDATA sndDataHandle, int vol, int pan) = 0;

	/*!
	 * \brief Plays a song
	 * \param modDataHandle Handle with the song data to play-
	 * \param masterVol volume of the song
	 * \param loop Specifies if you want loop or not
	 * \param firstOrder Where to start playing
	 * \param lastOrder Where to stop playing, or loop back to firstOrder if looping is enabled. -1 means play to the end.
	 * \return Returns the handler to control the song during playback, or -1 on error
	 * \sa \link Stop \endlink
	 */
	virtual HSOUND PlayModule(HMODULEDATA modDataHandle, int masterVol=AUDIO_MASTERVOL_DEFAULT, bool loop=false, int firstOrder=0, int lastOrder=-1) = 0;

#if CZMICROAUDIO_OGG_ENABLED
	virtual HSOUND PlayStream(HSTREAMDATA sndDataHandle, int vol=-1, bool loop=false) = 0;
#endif

	/*!
	 * \brief Stops a sound that is currently playing
	 * \param sndHandle Handle of the sound, which you'll get when you start playing the sound
	 * \return Error::Success on success, other on error.
	 * \sa \link Play \endlink \link PlayModule \endlink
	*/
	virtual int Stop(HSOUND sndHandle) = 0;

	/*!
	 * \brief Stop everything that is playing
	 * \return Error::Success on success, other on error.
	 */
	virtual int StopAll(void) = 0;

	/*!
	 * \brief Changes the volume of a currently playing sound
	 * \param sndHandle Handle of the sound, which you'll get when you start playing the sound
	 * \param vol Volume (0..255)
	 * \return Error::Success on success, other on error.
	 * \sa \link Play \endlink \link PlayModule \endlink \link PlayStream \endlink
	 */
	virtual int SetVolume(HSOUND sndHandle, uint8_t vol) = 0;

	/*!
	 * \brief Changes the frequency on a currently playing sound
	 * \param sndHandle Handle of the sound, which you'll get when you start playing the sound
	 * \param freq Frequency
	 * \return Error::Success on success, other on error.
	 *
	 * \warning This doesn't work when called on song handles
	 * \sa \link Play \endlink \link PlayModule \endlink \link PlayStream \endlink
	 */
	virtual int SetFrequency(HSOUND sndHandle, int freq) = 0;

	/*!
	 * \brief change the pan position (stereo position) of a playing sound.
	 * \param sndHandle Handle of the sound, which you'll get when you start playing the sound
	 * \param pan (0=left-most, 64=middle, 128=right-most)
	 * \return Error::Success on success, other on error.
	 * \warning This doesn't work when called on song handles
	 * \sa \link Play \endlink \link PlayModule \endlink \link PlayStream \endlink
	 */
	virtual int SetPanning(HSOUND sndHandle, uint8_t pan) = 0;

	/*!
	 * \brief Pauses a currently playing sound
	 * \param sndHandle Handle of the sound, which you'll get when you start playing the sound
	 * \return Error::Success on success, other on error.
	 * \sa \link Play \endlink \link PlayModule \endlink \link PlayStream \endlink
	 */
	virtual int Pause(HSOUND sndHandle) = 0;

	/*!
	 * \brief Resumes a paused sound.
	 *
	 * \param sndHandle Handle of the sound, which you'll get when you start playing the sound
	 * \return Error::Success on success, other on error.
	 * \sa \link Pause \endlink \link Play \endlink \link PlayModule \endlink
	 */
	virtual int Resume(HSOUND sndHandle) = 0;


	/*!
	 * \brief Specifies the object that will be notified of some playback events
	 * \returns The previous handler
	 */
	virtual AudioPlayerListener* SetListener(AudioPlayerListener *listener) = 0;

	/*!
	 * \brief Changes the master volume
	 * \param vol Master volume. From 0 to \link cz::microaudio::AUDIO_MASTERVOL_MAX \endlink
	 * \return Error::Success on success, other on error
 	*/
	virtual int SetSFXMasterVolume(int vol) = 0;

	/*!
	 * \brief Gets the master volume
 	 * \return Master volume currently used for sounds
	 */
	virtual int GetSFXMasterVolume(void) = 0;

		// For internal use only
		//virtual void* GetParent()=0;

protected:

private:

};


} // namespace cz::microaudio

