#ifndef battery_h
#define battery_h

#include <Arduino.h>

class BatteryMonitor {
  public:
    void begin();
    void update();
    bool available() const { return _available; }
    uint16_t millivolts() const { return _millivolts; }
    uint8_t percent() const;
    void format(char *buffer, size_t length) const;

  private:
    uint16_t _millivolts = 0;
    bool _available = false;
};

extern BatteryMonitor battery;

#endif
