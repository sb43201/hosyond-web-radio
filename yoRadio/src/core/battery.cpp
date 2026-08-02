#include "options.h"
#include "battery.h"

BatteryMonitor battery;

#if BATTERY_ADC_PIN != 255
namespace {
struct BatteryCurvePoint {
  uint16_t millivolts;
  uint8_t percent;
};

// Approximate resting-voltage curve for a protected single-cell Li-ion pack.
const BatteryCurvePoint batteryCurve[] = {
  { 3300,   0 }, { 3500,   5 }, { 3600,  10 }, { 3700,  25 },
  { 3800,  45 }, { 3900,  65 }, { 4000,  80 }, { 4100,  90 },
  { 4200, 100 }
};
}
#endif

void BatteryMonitor::begin() {
#if BATTERY_ADC_PIN != 255
  pinMode(BATTERY_ADC_PIN, INPUT);
  analogSetPinAttenuation(BATTERY_ADC_PIN, ADC_11db);
  update();
#endif
}

void BatteryMonitor::update() {
#if BATTERY_ADC_PIN != 255
  uint32_t adcMillivolts = 0;
  for (uint8_t sample = 0; sample < BATTERY_ADC_SAMPLES; ++sample) {
    adcMillivolts += analogReadMilliVolts(BATTERY_ADC_PIN);
    delayMicroseconds(250);
  }
  adcMillivolts /= BATTERY_ADC_SAMPLES;

  const uint32_t cellMillivolts =
    (adcMillivolts * BATTERY_DIVIDER_NUMERATOR + BATTERY_DIVIDER_DENOMINATOR / 2) /
    BATTERY_DIVIDER_DENOMINATOR;

  // Ignore a floating/empty battery input. Smooth valid readings to prevent
  // the percentage from flickering as audio and backlight current changes.
  _available = cellMillivolts >= BATTERY_PRESENT_MV;
  if (!_available) {
    _millivolts = 0;
  } else if (_millivolts == 0) {
    _millivolts = cellMillivolts;
  } else {
    _millivolts = (_millivolts * 3UL + cellMillivolts + 2) / 4;
  }
#endif
}

uint8_t BatteryMonitor::percent() const {
#if BATTERY_ADC_PIN != 255
  if (!_available) return 0;
  if (_millivolts <= batteryCurve[0].millivolts) return batteryCurve[0].percent;
  const size_t last = sizeof(batteryCurve) / sizeof(batteryCurve[0]) - 1;
  if (_millivolts >= batteryCurve[last].millivolts) return batteryCurve[last].percent;

  for (size_t index = 1; index <= last; ++index) {
    if (_millivolts <= batteryCurve[index].millivolts) {
      const BatteryCurvePoint &low = batteryCurve[index - 1];
      const BatteryCurvePoint &high = batteryCurve[index];
      return low.percent +
        static_cast<uint32_t>(_millivolts - low.millivolts) *
        (high.percent - low.percent) /
        (high.millivolts - low.millivolts);
    }
  }
#endif
  return 0;
}

void BatteryMonitor::format(char *buffer, size_t length) const {
  if (!buffer || length == 0) return;
  if (!_available) {
    buffer[0] = '\0';
    return;
  }
  snprintf(buffer, length, "%u%% %u.%02uV", percent(),
           _millivolts / 1000, (_millivolts % 1000) / 10);
}
