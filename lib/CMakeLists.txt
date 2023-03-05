set(SOURCE_FILES
	"library.json"
	"LICENSE"

	# Revised
	"src/crazygaze/microaudio/Config.h"
	"src/crazygaze/microaudio/Defs.h"
	"src/crazygaze/microaudio/Core.cpp"
	"src/crazygaze/microaudio/Core.h"
	"src/crazygaze/microaudio/LinkedList.h"
	"src/crazygaze/microaudio/MemoryTracker.cpp"
	"src/crazygaze/microaudio/MemoryTracker.h"
	"src/crazygaze/microaudio/Memory.h"
	"src/crazygaze/microaudio/PlayerPrivateDefs.h"
	"src/crazygaze/microaudio/Output.h"

	"src/crazygaze/microaudio/output/BaseOutput.h"
	"src/crazygaze/microaudio/output/win32/Win32WaveOutOutput.h"
	"src/crazygaze/microaudio/output/win32/Win32WaveOutOutput.cpp"
	"src/crazygaze/microaudio/output/win32/waveOutErrorCodes.txt"

	# Not revised
	"src/crazygaze/microaudio/AudioSource.h"
	"src/crazygaze/microaudio/AudioPlayer.h"
	"src/crazygaze/microaudio/Player.h"
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

