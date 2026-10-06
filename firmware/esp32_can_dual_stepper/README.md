# ESP32 CAN Dual-Stepper Controller

This folder contains the current bench-tested hardware-control milestone for the robotic arm project.

## Verified hardware chain

```text
SavvyCAN
   |
CANable 2.0
   |
CANH / CANL
   |
SN65HVD230
   |
ESP32
   |-------------------|
TB6600 #1          TB6600 #2
   |                   |
NEMA 23 #1         NEMA 23 #2
```

The system has been tested with individual motor commands and synchronized dual-motor commands.

## CAN configuration

- CAN bitrate: **250 kbps**
- ESP32 CAN TX: **GPIO18**
- ESP32 CAN RX: **GPIO21**
- Motor command CAN ID: **0x100**
- Standard 11-bit frame
- Data length: **1 byte**

In the tested SavvyCAN build, entering decimal **256** in the ID field corresponds to CAN ID **0x100**.

### Commands

| Data byte | Action |
|---|---|
| `01` | Motor 1, one revolution |
| `02` | Motor 2, one revolution |
| `03` | Both motors simultaneously, same commanded step rate, opposite directions |
| `04` | Both motors simultaneously with the differential direction reversed |

## ESP32 to TB6600 wiring

### Motor 1

```text
ESP32 GPIO25 -> TB6600 #1 PUL+
ESP32 GPIO26 -> TB6600 #1 DIR+
ESP32 GND    -> TB6600 #1 PUL-
ESP32 GND    -> TB6600 #1 DIR-
```

### Motor 2

```text
ESP32 GPIO32 -> TB6600 #2 PUL+
ESP32 GPIO33 -> TB6600 #2 DIR+
ESP32 GND    -> TB6600 #2 PUL-
ESP32 GND    -> TB6600 #2 DIR-
```

ENA is currently left unconnected.

## CAN transceiver wiring

```text
ESP32 GPIO18 -> SN65HVD230 CTX
ESP32 GPIO21 <- SN65HVD230 CRX
ESP32 3V3    -> SN65HVD230 3V3
ESP32 GND    -> SN65HVD230 GND

SN65HVD230 CANH -> CANable CANH
SN65HVD230 CANL -> CANable CANL
```

## Common-ground requirement

A critical bench-test finding was that the ESP32 control ground must be connected to the TB6600 signal return.

Without a common signal reference, the STEP/DIR voltage seen by the TB6600 can be effectively invalid even though the ESP32 GPIO is toggling. The observed symptom was that the stepper motors locked/buzzed instead of rotating.

Keep the ESP32, CAN transceiver, and TB6600 control returns referenced correctly.

## Microstepping

The current firmware is configured for **1/16 microstepping**:

```text
200 full steps/revolution x 16 = 3200 pulses/revolution
```

For the TB6600 units used in this project, the tested 1/16 switch setting is:

```text
SW1 = OFF
SW2 = ON
SW3 = OFF
```

Current-limit switches remain configured separately according to the motor/driver setup.

Always remove motor power before changing DIP switches.

## Motion profile

The firmware uses a cosine-interpolated delay ramp for smoother startup and stopping.

Current parameters:

```text
START_DELAY_US = 1200
MIN_DELAY_US   = 150
RAMP_STEPS     = 1000
```

At 1/16 microstepping, the minimum half-period delay corresponds to approximately:

```text
STEP frequency ~= 3333 pulses/s
motor speed    ~= 62.5 RPM
```

The two-motor function generates the STEP pulses for both drivers together, so the commanded pulse frequency is synchronized.

This is still an open-loop stepper system. Equal commanded pulse frequency does not guarantee equal physical shaft speed if a motor loses steps under load.

## Current limitation

The present implementation uses blocking `delayMicroseconds()` loops. While a motor move is executing, the ESP32 is not continuously servicing new CAN commands.

For the next hardware-control stage, this should be replaced with hardware-timer/RMT or another non-blocking step-generation architecture so that:

- CAN remains responsive during motion
- velocity and position commands can be updated continuously
- emergency stop/abort commands can be handled promptly
- multiple joints can be coordinated
- encoder feedback can be integrated

## Source

The current firmware is in:

```text
firmware/esp32_can_dual_stepper/esp32_can_dual_stepper.ino
```
