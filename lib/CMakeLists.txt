set(SOURCE_FILES
	"library.json"
	"LICENSE"

	"src/crazygaze/microaudio/config/CustomConfig.h"

	"src/crazygaze/microaudio/win32/Win32WaveOutOutput.cpp"
	"src/crazygaze/microaudio/win32/Win32WaveOutOutput.h"
	"src/crazygaze/microaudio/win32/waveOutErrorCodes.txt"

	"src/crazygaze/microaudio/All.h"
	"src/crazygaze/microaudio/Audio.h"
	"src/crazygaze/microaudio/AudioPlayer.h"
	"src/crazygaze/microaudio/AudioPlayerListener.h"
	"src/crazygaze/microaudio/Config.h"
	"src/crazygaze/microaudio/Core.cpp"
	"src/crazygaze/microaudio/Core.h"
	"src/crazygaze/microaudio/DiskFile.cpp"
	"src/crazygaze/microaudio/DiskFile.h"
	"src/crazygaze/microaudio/File.cpp"
	"src/crazygaze/microaudio/File.h"
	"src/crazygaze/microaudio/ITModule.cpp"
	"src/crazygaze/microaudio/ITModule.h"
	"src/crazygaze/microaudio/LinkedList.h"
	"src/crazygaze/microaudio/Logger.h"
	"src/crazygaze/microaudio/MemFile.cpp"
	"src/crazygaze/microaudio/MemFile.h"
	"src/crazygaze/microaudio/Memory.h"
	"src/crazygaze/microaudio/MemoryProvider.h"
	"src/crazygaze/microaudio/MemoryTracker.cpp"
	"src/crazygaze/microaudio/MemoryTracker.h"
	"src/crazygaze/microaudio/Mixer.cpp"
	"src/crazygaze/microaudio/Mixer.h"
	"src/crazygaze/microaudio/MixerListener.h"
	"src/crazygaze/microaudio/MODModule.cpp"
	"src/crazygaze/microaudio/MODModule.h"
	"src/crazygaze/microaudio/Module.cpp"
	"src/crazygaze/microaudio/Module.h"
	"src/crazygaze/microaudio/Object.cpp"
	"src/crazygaze/microaudio/Object.h"
	"src/crazygaze/microaudio/Player.h"
	"src/crazygaze/microaudio/PlayerPrivateDefs.h"
	"src/crazygaze/microaudio/SoundOutput.cpp"
	"src/crazygaze/microaudio/SoundOutput.h"
	"src/crazygaze/microaudio/StaticSound.cpp"
	"src/crazygaze/microaudio/StaticSound.h"
	"src/crazygaze/microaudio/Stream.cpp"
	"src/crazygaze/microaudio/Stream.h"
	"src/crazygaze/microaudio/WAVLoader.cpp"
	"src/crazygaze/microaudio/WAVLoader.h"
)

#source_group("crazygaze\\czmicroaudio" FILES ${SOURCE_FILES})
add_library(czmicroaudio STATIC ${SOURCE_FILES})
target_include_directories(czmicroaudio PUBLIC "./src")
if (MSVC)
	#target_link_libraries(czmicroaudio PRIVATE Shlwapi Winmm Iphlpapi.lib)
	target_link_libraries(czmicroaudio PRIVATE Winmm)
	target_link_libraries(czmicroaudio PRIVATE windowsHAL)
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})

add_subdirectory(./examples)

