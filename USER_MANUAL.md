# Hosyond ESP32 Web Radio User Manual

## 1. Overview

This firmware turns the Hosyond/LCDWiki E32R35T into a standalone Wi-Fi
internet radio. The touchscreen controls playback and the built-in web
interface manages Wi-Fi, stations, sound, display settings, and updates.

The board already contains a mono SC8002B speaker amplifier. Audio leaves the
ESP32 through its internal DAC on GPIO26, so an external MAX98357A or other
I2S amplifier is not needed.

## 2. What you need

- Hosyond/LCDWiki E32R35T
- USB-C **data** cable
- Windows, macOS, or Linux computer
- Visual Studio Code with the PlatformIO extension
- Passive speaker with a matching 1.25 mm 2-pin plug
- 2.4 GHz Wi-Fi network with internet access
- Optional FAT32 microSD card

Use a stable 5 V USB supply. A supply rated for at least 1 A is recommended;
2 A provides useful margin at higher speaker volume and while charging a
battery.

## 3. Speaker connection

With power disconnected, plug the speaker into the connector marked
`SPEAKER`.

The two speaker contacts are `SP+` and `SP-`. This is a bridge-tied amplifier
output:

- Connect one speaker wire to `SP+`.
- Connect the other speaker wire to `SP-`.
- Do **not** connect either speaker wire to GND.
- Do **not** connect headphones directly to this output.

An 8 ohm, 1 W speaker is a conservative starting choice. Begin testing at a
low volume.

## 4. Installing the firmware

### 4.1 Install the development tools

1. Install Visual Studio Code.
2. Open the Extensions view.
3. Search for `PlatformIO IDE`.
4. Install it and allow VS Code to restart if requested.
5. Open the `Web Radio` project folder.

PlatformIO reads `platformio.ini` and selects the
`hosyond_e32r35t` environment automatically.

### 4.2 Connect the board

1. Use a USB-C cable that supports data, not a charge-only cable.
2. Connect the E32R35T directly to the computer.
3. Wait for the USB serial device to appear.

The board uses a CH340 USB-to-serial converter. If no serial port appears,
install the CH340 driver appropriate for the operating system.

### 4.3 Build

From the PlatformIO sidebar, select:

```text
PROJECT TASKS
  hosyond_e32r35t
    General
      Build
```

Or run:

```powershell
pio run -e hosyond_e32r35t
```

### 4.4 Upload the firmware

Select `Upload` in the same PlatformIO task list, or run:

```powershell
pio run -e hosyond_e32r35t -t upload
```

### 4.5 Upload the web interface

This step is required for initial installation:

```powershell
pio run -e hosyond_e32r35t -t uploadfs
```

In VS Code this task may be shown as **Upload Filesystem Image**.

The first command installs the ESP32 program. The second installs the browser
interface in SPIFFS. If only the firmware is uploaded, the screen may operate
but the web pages will be missing.

### 4.6 If automatic upload fails

If the terminal remains at `Connecting...`:

1. Hold the `BOOT` button.
2. Start the upload.
3. Release `BOOT` when connection begins.
4. Press `RESET` after the upload finishes.

To list available ports:

```powershell
pio device list
```

If necessary, place the correct port in `platformio.ini`:

```ini
upload_port = COM5
monitor_port = COM5
```

Replace `COM5` with the port shown on the computer.

### 4.7 Serial diagnostics

Open the serial monitor at 115200 baud:

```powershell
pio device monitor -e hosyond_e32r35t
```

Press `Ctrl+C` to exit.

## 5. First startup and Wi-Fi setup

After flashing both images:

1. Press `RESET`.
2. Wait for the setup screen.
3. On a phone or computer, open the Wi-Fi settings.
4. Join:

   ```text
   SSID:     yoRadioAP
   Password: 12345987
   ```

5. Open `http://192.168.4.1/` in a browser if the setup page does not appear
   automatically.
6. Select a 2.4 GHz Wi-Fi network and enter its password.
7. Wait for the radio to restart and connect.

The ESP32 supports 2.4 GHz Wi-Fi only. A 5 GHz-only network will not appear.

After connection, the display shows the radio's local IP address. Open that
address from another device on the same network to reach the control page.

## 6. Touchscreen controls

The normal touch gestures are:

| Gesture | Action |
|---|---|
| Tap upper-right corner | Cycle Player → Stations → Favorites → Player |
| Hold upper-right corner in station list | Move to the previous page |
| Hold lower-right corner in station list | Move to the next page |
| Hold a highlighted station away from the right corners | Add or remove Favorite |
| Tap in player view | Start or stop playback |
| Double-tap the highlighted station | Play the selected station |
| Swipe left or right | Adjust volume |
| Swipe up or down | Browse stations |
| Long press | Change between player and station-list views or cancel selection |

The upper-right 96x64-pixel area is a dedicated page-switch shortcut in this
Hosyond build. Each short tap cycles through the player, full station list,
Favorites list, and back to the player.

While the station list is open, hold the upper-right corner to move backward
ten stations or hold the lower-right corner to move forward ten stations.
These page gestures are useful for navigating long playlists quickly.

### Favorites

In either station list, hold the highlighted station anywhere outside the two
right-side paging corners to add or remove it from Favorites. Favorite stations
are marked with `*`. The Favorites page supports normal swiping, ten-station
paging, and double-tap playback.

Favorites are stored by stream URL in `/data/favorites.txt`, not by station
number. They survive restarts and remain associated after the playlist is
reordered. Up to 64 favorite URLs are stored.

The playlist-to-favorite mapping is read once and cached in RAM. Changing to
the Favorites page does not repeatedly open the playlist files, which keeps
audio playback uninterrupted.

In the station list, the centered station is the highlighted selection. The
first tap leaves the list open; tap the same highlighted station again within
0.7 second to start it. A vertical swipe clears any pending first tap, so
releasing the screen will not accidentally play a station.

Volume follows horizontal finger travel directly: right increases volume and
left decreases it. Approximately half the screen width changes the volume by
half of its full range. You can lift and swipe again when a larger adjustment
is needed.

Because the E32R35T uses a resistive touchscreen:

- Use a deliberate, light press.
- A fingernail or the supplied stylus is more precise than a broad fingertip.
- Avoid hard pressure that could damage the panel.

## 7. Adding and managing stations

### Importing the combined playlist

The recommended import file is `playlists/yoRadio_playlist.csv`. It combines
77 China/Taiwan stations and 53 U.S. stations for a total of 130. In the
yoRadio web interface, open the playlist editor and use its import function to
upload that file. It is already in yoRadio's tab-separated `name`, `stream
URL`, and `enabled` format. This main-branch firmware does not include the
optional Chinese display font; use the Chinese-support branch if those station
names must be rendered on the radio screen.

1. Open the radio's IP address in a browser.
2. Open the playlist or station editor.
3. Add a station name and its direct stream URL.
4. Save the playlist.
5. Select the station on the touchscreen or web player.

Use a direct audio-stream address, not the station's normal website address.
Common working streams use MP3 or AAC over HTTP or HTTPS.

yoRadio can also import a KaRadio-compatible `WebStations.txt` list. Review
imported entries afterward because public station URLs sometimes change.

If a station does not play:

- Try another station to confirm that Wi-Fi and audio work.
- Verify that the URL is a direct stream.
- Check the serial monitor for HTTP, TLS, codec, or timeout messages.
- Prefer a moderate-bitrate MP3 or AAC stream when testing.

## 8. Volume and audio

Volume can be changed with a horizontal swipe or from the browser interface.
The onboard amplifier is enabled only while the player is active:

- GPIO4 LOW: amplifier enabled
- GPIO4 HIGH: amplifier shut down

The system produces mono output. The firmware forces stereo streams to mono so
both channels remain audible through the single onboard amplifier.

If sound is distorted:

1. Reduce the software volume.
2. Confirm the speaker is connected between `SP+` and `SP-`.
3. Use a speaker with a suitable impedance and power rating.
4. Use a stronger 5 V supply and a short USB cable.

## 9. Display and touch settings

### Time synchronization and daylight saving

This Hosyond build uses NTP and the U.S. Eastern Time rule. It changes
automatically between EST (UTC-5) and EDT (UTC-4) at 2:00 a.m. on the correct
March and November Sundays. The numeric UTC-offset field in the web interface
is ignored by this hardware profile; leave the configured NTP server enabled.
The displayed minute is refreshed from the ESP32 system clock rather than a
loop-based counter, preventing accumulated lag during audio or display work.

The web settings include display orientation, touch orientation, inversion,
brightness, and diagnostic options.

The initial touch limits are compiled into `yoRadio/myoptions.h`:

```cpp
#define TS_X_MIN 300
#define TS_X_MAX 3800
#define TS_Y_MIN 280
#define TS_Y_MAX 3850
```

If touch moves in the wrong direction, try **Flip touch** before changing these
values. Use **Flip screen** if the entire display orientation is reversed.

If touch positions remain inaccurate:

1. Enable touch debugging in the web settings.
2. Open the serial monitor.
3. Touch near all four edges and record the raw minimum and maximum values.
4. Update the four values in `yoRadio/myoptions.h`.
5. Rebuild and upload the firmware. The filesystem does not need to be
   re-uploaded for calibration-only changes.

### Weather display

The Hosyond layout pauses the weather ticker for three seconds before scrolling
at approximately 25 pixels per second. Current temperature and "feels like"
temperature are shown in both Fahrenheit and Celsius:

```text
72.5°F / 22.5°C
```

OpenWeather is queried in metric units; Fahrenheit is calculated locally by
the radio.

## 10. Using a microSD card

The onboard microSD slot is enabled. It uses a separate SPI bus from the TFT
and touch controller.

For best compatibility:

1. Power off the radio.
2. Format the card as FAT32.
3. Copy supported audio files to the card.
4. Insert the card fully.
5. Power the radio on.
6. Switch between web-radio and SD modes from the web controls or the
   appropriate player mode control.

Use short folder structures. The firmware's default scan depth is limited, so
deeply nested files may not appear.

Always stop playback or power off before removing the card.

## 11. Browser control

Any phone, tablet, or computer on the same local network can control the radio:

1. Open the IP address shown on the radio.
2. Use the player page for play, stop, volume, and station selection.
3. Use the settings pages for Wi-Fi, display, sound, time, and playlist
   configuration.

If the browser page does not load:

- Confirm the controlling device is on the same network.
- Use the numeric IP address rather than a hostname.
- Upload the filesystem image again if the page files are missing.
- Restart the router and radio if the assigned IP address has changed.

## 12. Updating the firmware

### USB update

Rebuild and upload:

```powershell
pio run -e hosyond_e32r35t -t upload
```

Upload the filesystem again when files under `yoRadio/data` have changed:

```powershell
pio run -e hosyond_e32r35t -t uploadfs
```

Uploading the filesystem can replace web-interface data. Back up playlists and
Wi-Fi configuration before a major upstream update.

### Over-the-air update

yoRadio supports OTA after the initial USB installation. OTA is convenient for
firmware-only updates, but use USB when recovering a board that no longer
connects to Wi-Fi.

## 13. Troubleshooting

### The board does not appear as a serial port

- Try another USB-C data cable.
- Try another USB port without a hub.
- Install the CH340 driver.
- Check Device Manager or `pio device list`.

### Upload stops at `Connecting...`

- Hold `BOOT` while the connection starts.
- Release it after PlatformIO begins writing.
- Disconnect other serial-monitor programs.
- Set the correct `upload_port`.

### The screen is dark

- Press `RESET`.
- Confirm a stable 5 V supply.
- Re-upload the firmware.
- Check that GPIO27 has not been reassigned; it controls the backlight.

### The screen is white, scrambled, or has wrong colors

- Confirm the build environment is `hosyond_e32r35t`.
- Do not flash the CYD ESP32-2432S028 configuration.
- Rebuild from this repository and upload again.

### Touch does not respond

- Confirm this is the E32R35T resistive-touch model.
- Check that GPIO33 remains touch CS and GPIO36 remains touch IRQ.
- Try **Flip touch**.
- Enable touch diagnostics and refine the calibration limits.

### There is no sound

- Confirm a speaker is connected to the `SPEAKER` socket.
- Never connect either speaker output to ground.
- Raise the volume from a low setting.
- Test several known working MP3 streams.
- Check the serial monitor for stream errors.
- Confirm GPIO26 and GPIO4 have not been repurposed.

### Sound clicks, breaks up, or buffers repeatedly

- Improve the 2.4 GHz Wi-Fi signal.
- Try a lower-bitrate stream.
- Use a stable 5 V supply.
- Reduce frequent screen or browser updates.
- Avoid forcing the optional framebuffer on this non-PSRAM board.

### Wi-Fi setup network does not appear

- Restart the radio.
- Wait at least 30 seconds.
- Confirm the phone has 2.4 GHz Wi-Fi enabled.
- Use the serial monitor to check startup status.

### Web interface returns an error or blank page

Re-upload SPIFFS:

```powershell
pio run -e hosyond_e32r35t -t uploadfs
```

Then press `RESET` and clear or refresh the browser cache.

## 14. Hardware reference

| Device | Connection |
|---|---|
| ST7796 TFT | HSPI: GPIO14/13/12, CS15, DC2 |
| XPT2046 touch | Shared HSPI, CS33, IRQ36 |
| Backlight | GPIO27 |
| SC8002B audio input | ESP32 DAC2, GPIO26 |
| SC8002B shutdown | GPIO4, active high shutdown |
| microSD | VSPI: GPIO18/23/19, CS5 |
| I2C connector | SDA32, SCL25 |

## 15. Safety and care

- Disconnect power before changing speaker or battery connections.
- Observe battery polarity exactly as marked.
- Use only a suitable 3.7 V lithium-polymer battery on the battery connector.
- Do not short `SP+` or `SP-` to ground.
- Do not press the resistive screen with sharp or hard objects.
- Keep the board away from conductive surfaces and liquids.
- Provide ventilation when operating at high speaker volume or charging a
  battery.

## 16. Software information

This firmware is based on
[e2002/yoRadio](https://github.com/e2002/yoradio). The Hosyond-specific settings
are maintained in `yoRadio/myoptions.h`, with the reproducible build definition
in `platformio.ini`.

The software is provided under the GNU General Public License v3.0. See
`LICENSE`.
