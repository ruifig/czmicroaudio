add_executable(simple
	"simple/simple.cpp"
)
target_link_libraries(simple czmicroaudio windowsHAL)
set_target_properties(simple PROPERTIES FOLDER "examples")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})

