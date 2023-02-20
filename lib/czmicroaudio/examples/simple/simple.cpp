#include <Arduino.h>
#include <czmicroaudio/Foo.h>

Foo foo;
void setup()
{
	Serial1.begin(115200);
	Serial1.println("Hello world!");
}

void loop()
{
	Serial1.println(foo.add(micros(), 0));
	delay(1000);
}
