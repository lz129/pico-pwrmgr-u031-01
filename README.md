# Bluetooth LE based Remote Control Transmitter

What is it? A replacement for broken remote controls used for garage doors and similar purposes. Original equipment is often expensive or completely unobtainium.

## Versions

This document describes rev A. The design and the PCB have some [issues](ISSUES.md) which requires hardware patches on the PCB.

## Hardware

The major part is a [Raspberry Pi Pico W](https://pip-assets.raspberrypi.com/categories/686-raspberry-pi-pico-w/documents/RP-008312-DS-1-pico-w-datasheet.pdf) which is responsible to handle the Bluetooth LE protocol. In order to handle the low power mode of the device an additional microcontroller is used.

The board has been designed with Kicad 10. For details refer to 
[Schematic](bt-rc-tx/schematic.pdf), [PCB](bt-rc-tx/board.pdf), and [PCB-3d](bt-rc-tx/board-3d.png).

Fixes required on the PCB rev A:

* The decoupling capacitor C2 for the microcontroller shall be changed from 100n to 10u. 

* Parallel to C1 an additional electrolyte cap with 470u has to be connected.

* Pin 39 of the Raspberry Pi Pico W has to be disconnected from the +BATT network and a jumper Jx has to be added.

Modifications required on the Raspberry Pi Pico W board:

* Remove R2 and R10 ([power circuit Pico W](images/power-circuit-pico-w.png))

Refer to [images](images) for details.

|Image|Content|
|-----|--------|
|[Shipping box](images/shipping-box.jpg)|PCBs as delivered by JLCPCB|
|[Delivered boards](images/delivered-boards.jpg)|PCBs as delivered by JLCPCB|
|[Board unpopulated (front)](images/board-unpopulated-front.jpg)|The unpopulated PCB from the top side|
|[Board unpopulated (back)](images/board-unpopulated-back.jpg)|The unpopulated PCB from the back side|
|[Breadboard setup](images/breadboard-setup.jpg)|The complete design on a breadboard|
|[Debug connections (front)](images/debug-connections-front.jpg)|Populated PCB with debug connections (front)|
|[Debug connections (back)](images/debug-connections-back.jpg)|Populated PCB with debug connections (back)|
|[Board in housing](images/board-in-housing.jpg)|Populated PCB in housing|
|[Final device with batteries](images/final-device-open-box.jpg)|Final device with batteries and open housing|



## Software

### Power Controller

The power controller, a STM32U031 microcontroller handles power up and power down of the Raspberry Pi Pico W, reads keyboard input and controls the LEDs. When completely powered down the transmitter draws only approx. 400nA from the Batteries.

The power controller shuts down the Raspi automatically after a given time (currently 10s) in case the Python script does not complete for whatever reason. The script is able to send a signal over GPIO to the power controller to request an immediate shutdown after its job has finished.

To assist development the power controller can be switched to a debug mode where the Raspberry Pi is powered without timeout. This mode shall be used for Python script development. The mode can be activated by pressing button A for more than 3s. Press button A again to leave the debug mode.

Another debug and test aid is the inhibit power down jumper J4. When active the power controller does not go into power down mode but behaves otherwise identical. Use this jumper when debugging the power controller with ST-Link.

__Note:__ Powering the Raspberry Pi Pico W without timeout is only possible when the jumper J4 is set.

__IMPORTANT:__ The jumper J5 (currently not in rev A design) MUST be removed when connecting the Raspberry Pi Pico W via USB cable to a PC. The cable would supply the power controller with the USB bus voltage of 5V. The power controller however is a 3.3V device which can be damaged in this mode. Instead connect the battery or a lab power supply with 3.3V to supply the power controller.

### Raspberry Pi Pico W

Timing and performance is not critical in the application. Therefore a solution with [Micropython](https://micropython.org) has been chosen. Prebuilt versions for the Raspberry Pi Pico W are available [here](https://micropython.org/download/RPI_PICO_W/). Installation of the Python system can be easily done without tools unsing the UF2 bootloader.

The current python [script](python/garagedoor_central.py) is minimalistic at this time and can be extended as necessary to add additional features.

__Attention__: The sample script has no means for secure communication!

### Connectors and Jumpers

All connectors and jumpers are located on the back side of the PCB.

__J1__ is the connector for the battery. The transmitter is designed to operate on two AA or AAA cells. Both alkali mangan and NiMH cells can be used. Note that the voltage range is dicated by the microcontroller which operates on 1.71 to 3.6 volts and the buck/boost converter in the Raspberry Pi Pico W module which accepts 1.8 to 5.5 volts. Therefore the effective voltage range is between 1.8 and 3.6 volts.

__J3__ is a debug connector to attach a debug probe like the ST-Link from ST Microelectronics. The pin assignment is as follows:

|Pin|Purpose|Comments|
|---|-------|--------|
|1|GND|
|2|VCC|This is a sense wire for the debug probe to handle the voltage levels correctly. Do not provide power on this pin!|
|3|SWDIO|Serial Wire Debug data I/O 
|4|SWCLK|Serial Wire Debug clock
|5|/Reset|Reset controller

__J5__ jumper to inhibit shutdown. Use it when debugging to avoid debug probe disconnect due to powering down the chip. For normal operation and low power while not using the device the jumper has to be removed.

__Jx__ jumper patched into the rev A design. It disconnects the incoming power from the USB to the microcontroller. __The microcontroller can be damaged when an USB cable is connected to the Raspberry Pi board and the jumper is not removed.__


