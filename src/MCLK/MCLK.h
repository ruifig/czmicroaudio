#pragma once

class MCLK {
  public:
    MCLK();
    virtual ~MCLK();

    bool setPin(pin_size_t pin);
    // set MCLK clock, based on audio sampling rate (e.g. 48000hz) and multiplier (typically 256, check specs)
    bool setFrequency(unsigned int newFreq, unsigned int newMultiplier);

    bool begin(unsigned int sampleRate, unsigned int multiplier ) {
      setFrequency(sampleRate, multiplier);
      return begin();
    }

    bool begin();
    void end();

  private:
    pin_size_t _pin;
    unsigned int _freq = 48000;
    unsigned int _multiplier = 256;

    bool _running;

    PIOProgram *_mclk;
    PIO _pio;
    int _sm;
};
