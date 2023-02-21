set(SOURCE_FILES
	"Arduino.h"
	"Arduino.cpp"
	"README.md"
)

#source_group("crazygaze\\czmicroaudio" FILES ${SOURCE_FILES})
add_library(windowsHAL STATIC ${SOURCE_FILES})
target_include_directories(windowsHAL PUBLIC "./")
# target_link_libraries(czmicroaudio somelibraries)
source_group("Source Files" FILES ${SOURCE_FILES})

