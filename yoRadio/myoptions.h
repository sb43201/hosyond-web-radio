#ifndef myoptions_h
#define myoptions_h

/*
 * Hosyond/LCDWiki E32R35T hardware profile
 *
 * Board:
 *   ESP32-WROOM-32E
 *   3.5-inch 320x480 ST7796 TFT (used as 480x320 landscape)
 *   XPT2046 resistive touch
 *   SC8002B onboard mono speaker amplifier
 *
 * This file intentionally contains all board-specific settings so upstream
 * yoRadio source files can remain unchanged.
 */

#define L10N_LANGUAGE EN

/* ST7796 TFT on the board's HSPI bus. */
#define DSP_MODEL DSP_ST7796
#define DSP_HSPI true
#define TFT_CS 15
#define TFT_DC 2
#define TFT_RST -1
#define BRIGHTNESS_PIN 27

/*
 * This ESP32-WROOM-32E module has no PSRAM. Keep the optional animation
 * framebuffer disabled to preserve heap for HTTPS streams and audio buffers.
 */
#define USE_FBUFFER false

/* XPT2046 touch shares HSPI (SCK 14, MOSI 13, MISO 12) with the TFT. */
#define TS_MODEL TS_MODEL_XPT2046
#define TS_HSPI true
#define TS_CS 33

/*
 * Starting calibration values from the known E32R35T hardware. If touch is
 * mirrored on a particular panel, use yoRadio's "Flip touch" setting. Raw
 * values can be displayed by enabling touch debugging in the web settings.
 */
#define TS_X_MIN 300
#define TS_X_MAX 3800
#define TS_Y_MIN 280
#define TS_Y_MAX 3850

/*
 * Hosyond usability shortcut: a tap in the upper-right corner switches
 * directly between the player and station-list screens. Normal taps elsewhere
 * retain yoRadio's play/stop and station-select behavior.
 */
#define TS_MODE_SWITCH_CORNER true
#define TS_MODE_SWITCH_WIDTH 96
#define TS_MODE_SWITCH_HEIGHT 64

/* Long-press right-side corners to move ten stations (one visible page). */
#define TS_STATION_PAGE_STEPS 10
#define TS_STATION_PAGE_CORNER_HEIGHT 64

/*
 * A station starts only after two taps on the same highlighted item within
 * this interval. This prevents a stray touch from closing the station list.
 */
#define TS_STATION_DOUBLE_TAP_MS 700

/*
 * Horizontal volume swipes are distance based. A full screen-width movement
 * changes the 0-254 yoRadio volume scale by this amount.
 */
#define TS_VOLUME_SWIPE_RANGE 254
#define TS_VOLUME_MIN_CHANGE 2

/* Show OpenWeather current and "feels like" values in Fahrenheit and Celsius. */
#define WEATHER_DUAL_UNITS true

/* U.S. Eastern Time: EST (UTC-5), automatically changing to EDT (UTC-4). */
#define TIMEZONE_TZ_RULE "EST5EDT,M3.2.0/2,M11.1.0/2"

/*
 * The E32R35T has a built-in 100k/100k BAT+ divider connected to GPIO34.
 * GPIO34 is ADC1, so battery measurement remains available while Wi-Fi runs.
 */
#define BATTERY_ADC_PIN 34
#define BATTERY_DIVIDER_NUMERATOR 2
#define BATTERY_DIVIDER_DENOMINATOR 1
#define BATTERY_ADC_SAMPLES 16
#define BATTERY_PRESENT_MV 2500

/* Use readable footer labels instead of yoRadio's private icon characters. */
#define STATUS_TEXT_LABELS true
#define HIDE_IP true

/* Maximum ESP32 2.4 GHz transmit power (19.5 dBm). */
#define WIFI_TX_POWER WIFI_POWER_19_5dBm
#define WIFI_ATTEMPTS 40
#define WIFI_STARTUP_CYCLES 3
#define WIFI_RECONNECT_INTERVAL_MS 4000
#define WIFI_LOCK_STRONGEST_BSSID true

/* Compact Simplified/Traditional Chinese font generated from the bundled playlist. */
#define CJK_SUBSET_FONT true

/*
 * Onboard audio:
 *   GPIO26 -> ESP32 DAC2 -> SC8002B AUDIO_IN
 *   GPIO4  -> SC8002B shutdown (LOW enabled, HIGH muted)
 *
 * I2S_DOUT remains non-255 so yoRadio selects its audioI2S player path. With
 * I2S_INTERNAL enabled, no external BCLK/LRC/DOUT wiring is used.
 */
#define I2S_INTERNAL true
#define I2S_INTERNAL_DAC_CHANNEL I2S_DAC_CHANNEL_LEFT_EN
#define I2S_DOUT 26
#define MUTE_PIN 4
#define MUTE_VAL HIGH
#define AUDIO_POP_MUTE_DELAY_MS 120
#define AUDIO_POP_UNMUTE_FRAMES 5
#define PLAYER_FORCE_MONO true
#define VS1053_CS 255

/* Onboard microSD uses the independent VSPI bus: SCK 18, MISO 19, MOSI 23. */
#define SDC_CS 5
#define SD_HSPI false

/* Exposed I2C connector on the E32R35T. */
#define I2C_SDA 32
#define I2C_SCL 25

/* The board's RGB LED is not used by the radio firmware. */
#define LED_BUILTIN 255

#endif
