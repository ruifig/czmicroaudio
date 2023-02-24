
/*===============================================================================================
CrazyGaze (http://www.crazygaze.com)
Author : Rui Figueira
Email  : rui@crazygaze.com

This example shows the basic use of czPlayer, in its simplest form, without any callbacks.
To make thing even more clean, it doesn't even checks for errors.

===============================================================================================*/

#include <Arduino.h>

void setup()
{
	Serial1.begin(115200);
	Serial1.println("Hello world!");
}

void loop()
{
	Serial1.println(micros());
	delay(1000);
}

