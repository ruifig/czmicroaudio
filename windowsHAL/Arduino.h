#pragma once

#include <iostream>

void delay(int ms);

unsigned long micros(void);

class HardwareSerial
{
public:
	void begin(unsigned long /*baud*/)
	{
	}

	template<typename T>
	void print(T val)
	{
		std::cout << val;
	}

	template<typename T>
	void println(T val)
	{
		std::cout << val << std::endl;
	}

};

extern HardwareSerial Serial;
extern HardwareSerial Serial1;
