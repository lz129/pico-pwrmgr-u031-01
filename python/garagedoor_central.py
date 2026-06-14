from micropython import const
import uasyncio as asyncio
import aioble
import bluetooth
import random
import struct
from machine import Pin
from bluetooth import UUID

_ENV_GARAGEDOOR_SERVICE_UUID = UUID(0xD000)
_ENV_GARAGEDOOR_COMMAND_UUID = UUID(0xD100)
_ENV_GARAGEDOOR_STATUS_UUID  = UUID(0xD101)

status_led = Pin("LED", Pin.OUT)
shutdown_out = Pin(8, Pin.OPEN_DRAIN, value=1)

led1_in = Pin(6, Pin.IN, Pin.PULL_UP)
led2_in = Pin(7, Pin.IN, Pin.PULL_UP)

cmd_characteristic = None

async def find_door_peripheral():
    # Scan for 5 seconds, in active mode, with very low interval/window (to
    # maximise detection rate).
    print("Search for door peripheral")
    async with aioble.scan(5000, interval_us=30000, window_us=30000, active=True) as scanner:
        async for result in scanner:
            if result.name() == "GarageDoorT" and _ENV_GARAGEDOOR_SERVICE_UUID in result.services():
                print("Found peripheral <{}> / {}".format(result.name(), _ENV_GARAGEDOOR_SERVICE_UUID))
                return result.device
    return None

async def central_task():
    device = await find_door_peripheral()
    if not device:
        print("Door peripheral not found")
        return

    try:
        print("Connecting to", device)
        connection = await device.connect()
        print("Connected")
    except asyncio.TimeoutError:
        print("Timeout during connection")
        return

    async with connection:
        
        # Retrieve services and characteristics from remote peripheral
        try:
            global cmd_characteristic
            door_service = await connection.service(_ENV_GARAGEDOOR_SERVICE_UUID)
            cmd_characteristic = await door_service.characteristic(_ENV_GARAGEDOOR_COMMAND_UUID)
            if led1_in.value() == 0:
                await cmd_characteristic.write(b'\x01')
            if led2_in.value() == 0:
                await cmd_characteristic.write(b'\x02')
            print("Characteristic written")
            await asyncio.sleep(2.0)
        except asyncio.TimeoutError:
            print("Timeout discovering services/characteristics")
            return
        print("Close connection, all done well")
        return True


async def main():
    print("-------------- Garage Door Central Application ---------------")
    t2 = asyncio.create_task(central_task())    
    res = await asyncio.gather(t2)
    print("Central application ended", res)
    if res[0]:
        print("Door operation successful")
    else:
        print("Door operation not successful")
    # request shutdown / powerdown of pico    
    shutdown_out.off()


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print('Interrupted')
    finally:
        # Clear retained state
        asyncio.new_event_loop()