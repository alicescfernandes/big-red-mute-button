from bleak import BleakClient, BleakScanner
from dotenv import load_dotenv
from controls.lock_control import LockControl

import asyncio
import logging
import sys
import os

load_dotenv()  # This line brings all environment variables from .env into os.environ

BATTERY_CHARACTERISTIC = os.environ['BATTERY_CHARACTERISTIC']
BUTTON_CHARACTERISTIC = os.environ['BUTTON_CHARACTERISTIC']
PASS_CODE = os.environ['PASS_CODE']
DEVICE_NAME = os.environ['DEVICE_NAME']

# Set up logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

class BleDevice():
    def __init__(self):
        self.client = None

    def turn_on(self):
        """
        Turn on the device (placeholder for actual implementation)
        """
        logger.info("Turning on the device...")
        
        async def main():
            await self.client.write_gatt_char(BUTTON_CHARACTERISTIC,b'\x01')

        asyncio.run(main())
        

    def turn_off(self):
        """
        Turn off the device (placeholder for actual implementation)
        """
        logger.info("Turning off the device...")

        async def main():

            await self.client.write_gatt_char(BUTTON_CHARACTERISTIC, b'\x00')

        asyncio.run(main())

    async def close(self):
        """
        Close the connection to the device
        """
        if self.client and self.client.is_connected:
            await self.client.disconnect()
            logger.info("Disconnected from the device")
        else:
            logger.info("No active connection to close")

    async def isOpen(self):
        """
        Check if the connection to the device is open
        """
        if self.client:
            return await self.client.is_connected()
        return False

    async def connect_with_context(self,address, onRead):
        """
        Connect to the device and set up notifications with a callback
        """
        self.client = BleakClient(address)
        try:
            await self.client.connect()
            logger.info("Connected to the device")
            await self.client.start_notify(BUTTON_CHARACTERISTIC, onRead)
            logger.info(f"Started notifications on {BUTTON_CHARACTERISTIC}")
        except Exception as e:
            logger.error(f"Failed to connect: {e}")
            await self.close()

        while self.client.is_connected:
            await asyncio.sleep(1)
        

    async def connect(self,address):
        """
        Connect to the device
        """
        self.client = BleakClient(address)
        try:
            await self.client.connect()
            logger.info("Connected to the device")

            # Keep the script running to receive notifications
                
        except Exception as e:
            logger.error(f"Failed to connect: {e}")
            await self.close()
        

    async def subscribe(self, address, onRead):
        await self.client.start_notify(address, onRead)

    async def get_address_for_name(self, name):
        devices = await self.list_devices()
        for d in devices:
            if(d.name == name):
                return d.address

    async def list_devices(self):
        """
        List all BLE devices
        """
        devices = await BleakScanner.discover()
        return devices

device_control = LockControl()

def callback_fn(char, byte_array):
    logger.debug(f"Notification from {char}: {byte_array}")
    status = int.from_bytes(byte_array)

    if(status == 1):   
        print("status")
        # device_control.lock()
        
if __name__ == "__main__":
    args = sys.argv[1:]

    async def main(args):
        device = BleDevice()
        address = await device.get_address_for_name("Alice")
        print("address",address)
        await device.connect(address)
        print("connected")
        # await device.subscribe(BATTERY_CHARACTERISTIC, callback_fn)

        #device_control.check_status(device.turn_on, device.turn_off, interval_ms=250)

        #while await device.isOpen():
        #    await asyncio.sleep(1)

    asyncio.run(main(args))