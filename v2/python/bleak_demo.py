import asyncio
import logging
from bleak import BleakClient


address = "34:85:18:03:BE:1A"
MODEL_NBR_UUID = "64f7c7ff-29e2-4df0-b63e-06b77b21aa5c"

# Set up logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

def callback_fn(char, byte_array):
    logger.debug(f"Notification from {char}: {byte_array}")
    

async def main(address):
    client = BleakClient(address)

    try:
        await client.connect()
        logger.info("Connected")
        
        await client.start_notify(MODEL_NBR_UUID, callback_fn)
        logger.debug(f"Started notifications on {MODEL_NBR_UUID}")
        
        # Keep the script running to receive notifications
        while client.is_connected:
            await asyncio.sleep(1)

    except Exception as e:
        logger.error(f"Error: {e}")

    finally:
        if client.is_connected:
            await client.stop_notify(MODEL_NBR_UUID)
            logger.debug(f"Stopped notifications on {MODEL_NBR_UUID}")
            await client.disconnect()
            logger.debug("Disconnected")

if __name__ == "__main__":
    asyncio.run(main(address))
