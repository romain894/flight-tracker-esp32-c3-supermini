# ESP32-C3-supermini flight tracker

## CLI setup

Install platformio with pip

## Build and upload

Build:

```bash
pio run
```

Upload:

```bash
pio run -t upload
```

If you have issue while trying to upload to the board:

  - Press BOOT button and keep pressed
  - Press briefly RST button
  - Release BOOT button

This should put the board in download mode again.

## WiFi

Set the WiFi SSID and password in the file `src/secrets.h`.


Romain THOMAS 2025
