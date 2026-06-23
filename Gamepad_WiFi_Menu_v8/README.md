# Gamepad_WiFi_Menu_v7

ESP32 WiFi gamepad firmware for sending joystick/channel data over UDP. Version 7 is based on v6 and adds channel subtrim and endpoint setup in the OLED channel setup menu.

## Hardware

- ESP32
- ADS1115 analog ADC for four joystick axes
- SSD1306 128x64 OLED display
- MPU6050 IMU
- Two joystick buttons: JOYS, JOYP
- Two switches/buttons: SWA, SWB

## Build Options

Configured in `Options.h`:

- `__LOG__`: enable Serial logs.
- `__OLED__`: enable SSD1306 OLED UI.
- `__SENSORS__`: enable MPU6050 sampling and testing display.
- `__OTA__`: enable Arduino OTA update mode.
- `__LEFT_HAND_THROTTLE__`: left-hand throttle mapping is enabled.
- `__RIGHT_HAND_THROTTLE__`: available but currently disabled.

## Runtime Modes

The firmware starts in control mode. Hold both sticks/switch condition used by the code to enter menu mode: left joystick X and Y below 1250 while SWA and SWB are pressed.

Main OLED menu:

1. SSID SCAN MODE
2. PID TUNING MODE
3. CHANNEL SETUP
4. MODEL SETUP
5. RATE/EXPO SETUP
6. TESTING MODE
7. CALIBRATION MODE
8. EXIT

Navigation uses the right joystick Y axis. Confirm/back uses the right joystick X axis.

## Control Output

Control mode reads four joystick axes and sends an 8-channel UDP packet about every 10 ms.

The transmitter starts locked. CH3 throttle is forced to 1000 until the radio is armed.

- Arm/disarm: hold both joystick buttons with throttle low.
- Throttle cut: press SWA. This immediately disarms and forces CH3 to 1000.
- Dual rate: press SWB to use the low-rate profile; release it for high rate.

Target:

- IP: `192.168.4.1`
- UDP port: `6188`

Packet format:

```text
SRV%04d%04d%04d%04d%04d%04d%04d%04d
```

Channel source mapping:

- CH1: Rudder, left joystick X when `__LEFT_HAND_THROTTLE__` is enabled
- CH2: Throttle, left joystick Y when `__LEFT_HAND_THROTTLE__` is enabled
- CH3: Elevator, right joystick Y when `__LEFT_HAND_THROTTLE__` is enabled
- CH4: Aileron, right joystick X when `__LEFT_HAND_THROTTLE__` is enabled
- CH5-CH8: fixed at 1500 before channel configuration is applied

All channel outputs are clamped to 1000-2000.

## Channel Setup

Version 7 supports setup items for CH1-CH8:

- `REV`: normal/reversed channel direction.
- `RATE`: overall channel rate, 30%-125%, adjusted in 5% steps.
- `SUB`: subtrim, -250 to +250 microseconds, adjusted in 5 us steps.
- `LOW`: low-side endpoint, 30%-125%, adjusted in 5% steps.
- `HIGH`: high-side endpoint, 30%-125%, adjusted in 5% steps.
- `SAVE & EXIT`: writes the channel setup to EEPROM and returns to control mode.

Channel processing order:

1. Convert input around center 1500.
2. Apply reverse.
3. Apply rate.
4. Apply low or high endpoint depending on signal direction.
5. Add subtrim.
6. Clamp final output to 1000-2000.

Channel settings are stored per model in EEPROM with a versioned header. Existing single-profile channel data is used as a fallback for the first run after upgrading.

## Model Setup

The firmware supports three model memories. Each model stores its own channel setup and rate/expo setup. Switching models reloads the selected model profile immediately.

## Rate / Expo Setup

Each model stores:

- `LOW RATE`: used while SWB is pressed.
- `HIGH RATE`: used while SWB is released.
- `EXPO`: softens stick response around center while keeping full travel.

## WiFi Setup

The firmware runs in WiFi station mode and connects to a selected AP.

SSID scan accepts SSIDs with these prefixes:

- `Wright`
- `Hover`
- `Drone`

When an SSID is selected, the OLED opens a password editor. Use right joystick Y to change the current character, right joystick X to move forward/back, and SWB to save and connect. WiFi credentials are stored in EEPROM with a versioned header.

## PID Tuning

PID tuning mode edits nine PID values:

- Roll: P, I, D
- Pitch: P, I, D
- Yaw: P, I, D

Step sizes:

- 10
- 50
- 500

The PID save action sends:

```text
PID + nine 5-digit values
```

PID values are also stored in EEPROM.

## Testing Mode

Testing mode displays:

- SWA/SWB state
- Left joystick button and X/Y values
- Right joystick button and X/Y values
- MPU6050 estimated X/Y/Z device angles

## Calibration Mode

Calibration mode records full axis travel:

1. Center all sticks and press SWB.
2. Move all stick axes through their full travel.
3. Press SWB again to save min/center/max calibration values.

Calibration values are stored in EEPROM and loaded at boot.

## OTA Update

If `__OTA__` is enabled, OTA mode is enabled at boot when SWA and SWB are both pressed.

- Hostname: `ESP32-Gamepad`
- Password: `12345678`

## Known Limitations

- CH5-CH8 have configurable output processing, but no physical input source yet.
- WiFi password entry is not implemented; selected SSID is reused as the password.
- MPU6050 data is displayed in testing mode but is not mapped to a control channel.
- PID values are edited and transmitted, but this firmware does not run a local closed-loop flight controller.
- Failsafe behavior is expected to be handled by the receiver or bridge side.
