# Hosyond 3.5-inch ESP32 Web Radio

Internet radio firmware for the Hosyond/LCDWiki E32R35T touchscreen, based on
[e2002/yoRadio](https://github.com/e2002/yoradio).

The project uses the hardware already fitted to the E32R35T:

- ESP32-WROOM-32E with 2.4 GHz Wi-Fi
- 3.5-inch 320x480 ST7796 TFT, operated in 480x320 landscape mode
- XPT2046 resistive touchscreen
- SC8002B onboard mono speaker amplifier
- Onboard microSD card slot
- USB-C programming and power
- Optional 3.7 V lithium-polymer battery connection and charging circuit

No external I2S amplifier is required. The ESP32 internal DAC on GPIO26 feeds
the onboard SC8002B amplifier. Connect a passive speaker to the board's
`SPEAKER` socket.

> **Speaker safety:** the speaker socket is a bridge output. Connect the speaker
> only between `SP+` and `SP-`. Do not connect either speaker terminal to GND.

## Features

- MP3 and AAC internet-radio playback
- Touchscreen station selection, volume, play, and stop
- Browser-based setup and control
- Editable and importable station playlists
- Wi-Fi setup access point for first startup
- Station name, stream title, bitrate, clock, and signal display
- Optional FAT32 microSD playback
- Backlight brightness control
- Over-the-air firmware updates after initial USB installation

## Required hardware

- Hosyond/LCDWiki E32R35T board
- USB-C data cable
- Stable 5 V USB supply
- Passive speaker with a 1.25 mm 2-pin plug
  - An 8 ohm, 1 W speaker is a conservative starting choice
- Optional FAT32 microSD card

## Quick start

Install [Visual Studio Code](https://code.visualstudio.com/) and the PlatformIO
extension, open this repository, and connect the board by USB-C.

Build:

```powershell
pio run -e hosyond_e32r35t
```

Upload the firmware:

```powershell
pio run -e hosyond_e32r35t -t upload
```

Upload the required web interface:

```powershell
pio run -e hosyond_e32r35t -t uploadfs
```

Press `RESET` after both uploads. On first startup, connect a phone or computer
to:

```text
Network:  yoRadioAP
Password: 12345987
Address:  http://192.168.4.1/
```

Enter the credentials for a 2.4 GHz Wi-Fi network. The radio's assigned IP
address is displayed after it connects.

See the [User Manual](USER_MANUAL.md) for detailed installation, operation,
playlist management, microSD use, and troubleshooting.

## Board configuration

The board-specific profile is
[`yoRadio/myoptions.h`](yoRadio/myoptions.h). It keeps the Hosyond changes
separate from upstream yoRadio source.

| Function | GPIO |
|---|---:|
| TFT CS | 15 |
| TFT DC/RS | 2 |
| TFT SCK | 14 |
| TFT MOSI | 13 |
| TFT MISO | 12 |
| TFT backlight | 27 |
| Touch CS | 33 |
| Touch IRQ | 36 |
| Internal DAC / amplifier input | 26 |
| Amplifier shutdown | 4, LOW enables |
| microSD CS | 5 |
| microSD SCK | 18 |
| microSD MISO | 19 |
| microSD MOSI | 23 |

The TFT and touchscreen share HSPI. The microSD card uses the independent VSPI
bus.

## Project structure

| Path | Purpose |
|---|---|
| `platformio.ini` | Reproducible PlatformIO build environment |
| `yoRadio/myoptions.h` | Hosyond display, touch, audio, and SD configuration |
| `yoRadio/src/` | yoRadio application source |
| `yoRadio/data/` | Web interface uploaded to SPIFFS |
| `USER_MANUAL.md` | Installation and operating instructions |
| `HOSYOND_BUILD.md` | Concise hardware and developer build notes |

## Verified build

The `hosyond_e32r35t` PlatformIO environment has been compiled successfully
with:

- Espressif32 platform 6.10.0
- Arduino-ESP32 core 2.0.17
- Adafruit GFX 1.12.6
- XPT2046_Touchscreen 1.4.0

At the time of verification:

- Program flash: 1,435,389 bytes of 3,145,728 bytes (45.6%)
- Static RAM: 66,708 bytes of 327,680 bytes (20.4%)
- SPIFFS web-interface image: 917,504 bytes

## Upstream and license

This is a hardware-specific adaptation of
[e2002/yoRadio](https://github.com/e2002/yoradio). The upstream application and
this derivative are distributed under the
[GNU General Public License v3.0](LICENSE).
