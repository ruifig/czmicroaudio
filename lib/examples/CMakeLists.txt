add_executable(arduino_simple
	"arduino/simple/simple.cpp"
)
target_link_libraries(arduino_simple czmicroaudio windowsHAL)
set_target_properties(arduino_simple PROPERTIES FOLDER "examples/arduino")
cz_setCommonBinaryProperties(arduino_simple "/")

add_executable(win32_simple
	"win32/simple/win32_simple.cpp"
)
target_link_libraries(win32_simple czmicroaudio windowsHAL)
set_target_properties(win32_simple PROPERTIES FOLDER "examples/win32")
cz_setCommonBinaryProperties(win32_simple "/")

