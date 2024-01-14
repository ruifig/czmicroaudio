#include <Arduino.h>
#include "MCLK.h"
#include "mclk.pio.h"


MCLK::MCLK() {
  _running = false;
  _pin = 29;
  _freq = 48000;
}

MCLK::~MCLK() {
}

bool MCLK::setPin(pin_size_t pin) {
  if (_running) {
    return false;
  }
  _pin = pin;
  return true;
}

bool MCLK::setFrequency(unsigned int newFreq, unsigned int newMultiplier) {
  _freq = newFreq;
  _multiplier = newMultiplier;
  if (_running) {
    float bitClk = _freq * _multiplier * 2.0 /* edges per clock */;
    pio_sm_set_clkdiv(_pio, _sm, (float)clock_get_hz(clk_sys) / bitClk);
  }
  return true;
}


bool MCLK::begin() {
  _running = true;
  int off = 0;
  _mclk = new PIOProgram(&mclk_program);
  _mclk->prepare(&_pio, &_sm, &off);
  mclk_program_init(_pio, _sm, off, _pin);
  setFrequency(_freq, _multiplier);
  pio_sm_set_enabled(_pio, _sm, true);
  return true;
}

void MCLK::end() {
  _running = false;
  delete _mclk;
  _mclk = nullptr;
}
