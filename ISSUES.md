# Rev B
(not yet defined)

## Findings

# Rev A

## Findings

* Not all microUSB connectors fit into the Pico USB socket due to a missing cutout in the PCB.
<br>
__Workaround:__ Try to find a suitable USB cable or cut the area in the PCB around the USB socket.

* The VSYS of the Pico is connected to the battery which is necessary when the device is supplied from a 3V battery. With the USB cable attached to the Pico for programming and debugging purposes VSYS is approx. 5V (-0.4V due to serial Schottky diode) which can destroy the power controller STM32U031.
<br>
__Workaround:__ This connection needs to be cut on the board for programming and test purposes and later connected when the device is powered by its batteries.


