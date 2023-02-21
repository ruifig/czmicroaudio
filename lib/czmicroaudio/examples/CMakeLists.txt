add_executable(simple
	"simple/simple.cpp"
)
target_link_libraries(simple czmicroaudio windowsHAL)
set_target_properties(simple PROPERTIES FOLDER "examples")



