set(SOURCE_FILES
	"mymain.cpp"

	"UnitTests/LinkedListUnitTests.cpp"
	"UnitTests/MemoryUnitTests.cpp"
)

add_executable(czmicroaudio_unittests ${SOURCE_FILES})
target_link_libraries(czmicroaudio_unittests czmicroaudio windowsHAL czmut)
cz_setCommonBinaryProperties(czmicroaudio_unittests "/")

