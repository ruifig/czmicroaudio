set(SOURCE_FILES
	"src/czmicroaudio/Foo.cpp"
	"include/czmicroaudio/Foo.h"
)

#source_group("crazygaze\\czmicroaudio" FILES ${SOURCE_FILES})
add_library(czmicroaudio STATIC ${SOURCE_FILES})
target_include_directories(czmicroaudio PUBLIC "./include")
# target_link_libraries(czmicroaudio somelibraries)

