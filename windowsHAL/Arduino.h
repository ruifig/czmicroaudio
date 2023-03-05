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

	operator bool() const
	{
		return true;
	}

};

extern HardwareSerial Serial;
extern HardwareSerial Serial1;

using __FlashStringHelper = char;
#define F(string_literal) string_literal
#define PROGMEM

