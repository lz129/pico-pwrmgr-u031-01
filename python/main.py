import uasyncio as asyncio
import garagedoor_central

try:
    asyncio.run(garagedoor_central.main())
except KeyboardInterrupt:  
    print('Interrupted')
finally:
    # Clear retained state
    asyncio.new_event_loop()
    print("Application ended")