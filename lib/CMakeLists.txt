set(SOURCE_FILES
	"library.json"
	"LICENSE"

	"src/crazygaze/microaudio/globals/czITModuleTables.h"
	"src/crazygaze/microaudio/globals/czMODConsts.h"
	"src/crazygaze/microaudio/globals/czMODModuleTables.h"

	"src/crazygaze/microaudio/win32/czWin32WaveOutOutput.cpp"
	"src/crazygaze/microaudio/win32/czWin32WaveOutOutput.h"
	"src/crazygaze/microaudio/win32/waveOutErrorCodes.txt"

	"src/crazygaze/microaudio/czAll.h"
	"src/crazygaze/microaudio/czAudio.cpp"
	"src/crazygaze/microaudio/czAudio.h"
	"src/crazygaze/microaudio/czAudioPlayer.h"
	"src/crazygaze/microaudio/czAudioPlayerListener.h"
	"src/crazygaze/microaudio/czCore.cpp"
	"src/crazygaze/microaudio/czCore.h"
	"src/crazygaze/microaudio/czDiskFile.cpp"
	"src/crazygaze/microaudio/czDiskFile.h"
	"src/crazygaze/microaudio/czFile.cpp"
	"src/crazygaze/microaudio/czFile.h"
	"src/crazygaze/microaudio/czITModule.cpp"
	"src/crazygaze/microaudio/czITModule.h"
	"src/crazygaze/microaudio/czLogger.h"
	"src/crazygaze/microaudio/czMemFile.cpp"
	"src/crazygaze/microaudio/czMemFile.h"
	"src/crazygaze/microaudio/czMemoryProvider.h"
	"src/crazygaze/microaudio/czMixer.cpp"
	"src/crazygaze/microaudio/czMixer.h"
	"src/crazygaze/microaudio/czMixerListener.h"
	"src/crazygaze/microaudio/czMODModule.cpp"
	"src/crazygaze/microaudio/czMODModule.h"
	"src/crazygaze/microaudio/czModule.cpp"
	"src/crazygaze/microaudio/czModule.h"
	"src/crazygaze/microaudio/czObject.cpp"
	"src/crazygaze/microaudio/czObject.h"
	"src/crazygaze/microaudio/czPlatformHeaders.h"
	"src/crazygaze/microaudio/czPlayer.h"
	"src/crazygaze/microaudio/czPlayerConfig.h"
	"src/crazygaze/microaudio/czPlayerPrivateDefs.h"
	"src/crazygaze/microaudio/czSampler.cpp"
	"src/crazygaze/microaudio/czSampler.h"
	"src/crazygaze/microaudio/czSoundOutput.cpp"
	"src/crazygaze/microaudio/czSoundOutput.h"
	"src/crazygaze/microaudio/czStaticSound.cpp"
	"src/crazygaze/microaudio/czStaticSound.h"
	"src/crazygaze/microaudio/czStream.cpp"
	"src/crazygaze/microaudio/czStream.h"
	"src/crazygaze/microaudio/czUtils.h"
	"src/crazygaze/microaudio/czWAVLoader.cpp"
	"src/crazygaze/microaudio/czWAVLoader.h"

)

#source_group("crazygaze\\czmicroaudio" FILES ${SOURCE_FILES})
add_library(czmicroaudio STATIC ${SOURCE_FILES})
target_include_directories(czmicroaudio PUBLIC "./src")
if (MSVC)
	#target_link_libraries(czmicroaudio PRIVATE Shlwapi Winmm Iphlpapi.lib)
	target_link_libraries(czmicroaudio PRIVATE Winmm)
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})

add_subdirectory(./examples)
