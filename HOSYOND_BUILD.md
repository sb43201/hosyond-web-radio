# Hosyond E32R35T Web Radio

For complete installation and operating instructions, see
[USER_MANUAL.md](USER_MANUAL.md). The repository overview and quick-start
instructions are in [README.md](README.md).

This project is a board-specific build of
[yoRadio](https://github.com/e2002/yoradio) for the Hosyond/LCDWiki E32R35T
3.5-inch ESP32 touchscreen.

## Hardware

- Hosyond/LCDWiki E32R35T
- Passive speaker with a 1.25 mm 2-pin plug (an 8 ohm, 1 W speaker is a
  conservative starting choice)
- USB-C 5 V power supply
- Optional FAT32 microSD card

No external MAX98357A is required. The E32R35T contains an SC8002B mono
amplifier. The ESP32 internal DAC on GPIO26 feeds the amplifier, and GPIO4
enables it while audio is playing.

Connect the speaker to the board connector marked `SPEAKER`. Its two contacts
are the bridge outputs `SP+` and `SP-`. Do not connect either speaker wire to
ground.

## Board pin map

| Function | GPIO |
|---|---:|
| TFT CS | 15 |
| TFT DC/RS | 2 |
| TFT SCK | 14 |
| TFT MOSI | 13 |
| TFT MISO | 12 |
| Backlight | 27 |
| Touch CS | 33 |
| Touch IRQ | 36 |
| Internal DAC / amplifier input | 26 |
| Amplifier shutdown | 4 (LOW enables) |
| microSD CS | 5 |
| microSD SCK | 18 |
| microSD MISO | 19 |
| microSD MOSI | 23 |

## Build and upload

Install VS Code and PlatformIO, open this folder, and connect the E32R35T by
USB-C.

Build:

```text
pio run -e hosyond_e32r35t
```

Upload the firmware and web interface:

```text
pio run -e hosyond_e32r35t -t upload
pio run -e hosyond_e32r35t -t uploadfs
```

Open the serial monitor at 115200 baud:

```text
pio device monitor -e hosyond_e32r35t
```

## Regenerate the Chinese playlist font

The committed font contains only the characters used by the bundled
China/Taiwan playlist. After editing its Chinese station names, regenerate the
font before building:

```text
python tools/generate_cjk_font.py playlists/yoRadio_China_Taiwan_playlist.csv yoRadio/src/displays/fonts/chinese_playlist_font.h
```

The generator uses Pillow and the lighter Windows SimSun font by default. Its
default threshold is tuned for thin strokes on the Hosyond display; pass
`--threshold` with a higher value for still thinner glyphs.

## First startup

1. Power the radio and wait for the setup access point to appear.
2. Join `yoRadioAP` using password `12345987`.
3. Open `http://192.168.4.1/`.
4. Enter the 2.4 GHz Wi-Fi credentials.
5. Open the IP address shown on the display and add or import stations.

The web files must be uploaded with `uploadfs`. If the screen works but the web
page is missing, repeat that step.

## Touch operation

- Tap the upper-right corner: switch directly between player and station list.
- Tap in player view: play/stop. Double-tap the highlighted station to select.
- Swipe left/right: responsive distance-based volume control.
- Swipe up/down: station list.
- Long press: switch between player and station-list screens.

The initial raw touch limits are in `yoRadio/myoptions.h`. Enable touch debugging
in the yoRadio web settings if a particular panel needs slightly different
limits. The "Flip touch" and "Flip screen" settings handle reversed orientation.

The custom ST7796 layout in
`yoRadio/src/displays/conf/displayST7796conf_custom.h` slows the weather ticker
and pauses it before scrolling. `WEATHER_DUAL_UNITS` in `yoRadio/myoptions.h`
shows both Fahrenheit and Celsius.

## Upstream

The application source is maintained by the
[e2002/yoradio](https://github.com/e2002/yoradio) project and is licensed under
GPL-3.0. Board-specific changes in this repository are kept in
`yoRadio/myoptions.h` and `platformio.ini`.
