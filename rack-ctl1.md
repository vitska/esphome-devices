DEvice hardware design:
Board: ESP32-S3-ETH-CAM-KIT

| Peripheral | Component Pin | ESP32-S3 Pin | Function | Notes |
|---|---|---|---|---|
| 2.42" OLED (I2C) | GND | GND | Power Ground | Common Ground |
| | VCC | VSYS | Power Supply | 5V Rail |
| | SCL / SCK | GPIO46 | I2C Clock | Dedicated I2C Bus |
| | SDA / MOSI | GPIO45 | I2C Data | Dedicated I2C Bus |
| Rotary Encoder | CLK (A) | GPIO16 | Phase A | Hardware Interrupt / Pulse |
| | DT (B) | GPIO17 | Phase B | Phase Direction |
| | SW | GPIO15 | Push Button | Active LOW |
| Buttons | Button 1 | GPIO18 | Input Pin | Screen 1 Select (Active LOW) |
| | Button 2 | GPIO21 | Input Pin | Screen 2 Select (Active LOW) |
| | Button 3 | GPIO47 | Input Pin | Screen 3 Select (Active LOW) |
| | Button 4 | GPIO48 | Input Pin | Screen 4 Select (Active LOW) |
| PWM Fan 1 | Control PWM | GPIO38 | 25 kHz PWM Output | Standard PC Fan PWM |
| | Tachometer IN | GPIO1 | Pulse Counter Input | 2 Pulses/Rev -> RPM |
| PWM Fan 2 | Control PWM | GPIO2 | 25 kHz PWM Output | Moved from GPIO0 (Boot-safe pin) |
| | Tachometer IN | GPIO41 | Pulse Counter Input | Moved from GPIO3 (Boot-safe pin) |
| Direct LEDs | LED 1 | GPIO39 | Digital / PWM Output | Active HIGH |
| | LED 2 | GPIO40 | Digital / PWM Output | Active HIGH |
| Ethernet (Onboard W5500) | CLK | GPIO14 | SPI Clock | Reserved internal |
| | MOSI | GPIO11 | SPI Master Out | Reserved internal |
| | MISO | GPIO12 | SPI Master In | Reserved internal |
| | CS | GPIO10 | SPI Chip Select | Reserved internal |
| | INT | GPIO9 | Interrupt Pin | Reserved internal |
| | RST | GPIO13 | Reset Pin | Reserved internal |

The UI prompt:
create LCD UI: The buttons switching between 4 screens 
1. 2 FUN / TEMPERATURE  FAST FUN ANIMATION:
   T1: XX.X T2: XX.X T3: XX.X T4: XX.X T: XX.X
   F1: XXX% [XXXX-----] RPM FUN ANIMATION
   F2: XXX% [XXXX-----] RPM FUN ANIMATION
2. POWER INPUT INA3221
3. POWER OUTPUT 12V INA3221
4. POWER OUTPUT 5V INA3221

Rotary encoder push enters menu
TEMPERATURE LOW 20 - 100
TEMPERATURE HIGH 20 - 100 LOW < HIGH
FUN 1 PWM LOW 0% - 100%
FUN 1 PWM HIGH 0% - 100% LOW < HIGH
FUN 2 PWM LOW 0% - 100%
FUN 2 PWM HIGH 0% - 100% LOW < HIGH