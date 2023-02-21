#include <Arduino.h>
#include <chrono>
#include <stdio.h>

HardwareSerial Serial;
HardwareSerial Serial1;

void delay(int ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

struct MicrosImpl
{
	MicrosImpl()
	{
		startTime = std::chrono::high_resolution_clock::now();
	}

	std::chrono::high_resolution_clock::time_point startTime;

	unsigned long micros()
	{
		auto now = std::chrono::high_resolution_clock::now();
		//auto elapsed = std::chrono::duration_cast<std::chrono::duration<unsigned long, std::micro>>(now - startTime);
		auto elapsed = std::chrono::duration_cast<std::chrono::duration<unsigned long, std::micro>>(now - startTime);
		return elapsed.count();
	}
};

// By creating it as global object, the constructor will get the time at which the program started running, so we can
// then calculate for how long that program has been running when calling micros()
MicrosImpl gMicrosImpl;

unsigned long micros(void)
{
	return gMicrosImpl.micros();
}


extern void setup();
extern void loop();

int main(void)
{
	setup();
	while (true)
	{
		loop();
	}

	return EXIT_SUCCESS;
}

