import rumps
from ble_device import BleDevice
from utils import threaded 
from controls import lock_control
import asyncio
import threading
import time

class MuteButtonWidget(object):
    def __init__(self):
        self.config = {
            "app_name": "Mute Widget",
        }
        self.devices_list = {}

        self.loop = asyncio.new_event_loop()
        threading.Thread(target=self.loop.run_forever, daemon=True).start()
        self.device = BleDevice(self.loop)

        self.app = rumps.App(self.config["app_name"])
        self.set_up_menu()

        self.list_devices = rumps.MenuItem(title="Devices List", callback=self.on_tick)
        self.close_conn = rumps.MenuItem(title="Disconnect Device")

        self.app.menu = [self.list_devices, self.close_conn]
        self.timer = rumps.Timer(self.on_tick, 30)
        self.timer.start()

    def set_up_menu(self):
        self.app.title = "🔴"

    def on_tick(self, sender):
        future = asyncio.run_coroutine_threadsafe(self.device.list_devices(), self.loop)
        try:
            devices = future.result()
        except Exception as e:
            print("BLE Error", "Device scan failed", str(e))
            return

        self.devices_list = {}

        if len(self.list_devices):
            self.list_devices.clear()

        for device in devices:
            device_name = device.name or "Unnamed"
            self.devices_list[device_name] = device.address
            device_m = rumps.MenuItem(title=device_name, callback=self.connect_to_device)
            self.list_devices.add(device_m)
            
            
    def connect_to_device(self, sender):
        device_name = sender.title
        device_addr = self.devices_list.get(device_name)
        if not device_addr:
            print("Connection Error", "Device not found in list", sender.title)
            return

        future = asyncio.run_coroutine_threadsafe(self.device.connect(device_addr), self.loop)
        try:
            future.result(timeout=5)
        except Exception as e:
            print("BLE Error", "Failed to connect", str(e))
            return

        self.close_conn.set_callback(self.disconnect_device)
        self.lock_control()

    def disconnect_device(self, sender):
        future = asyncio.run_coroutine_threadsafe(self.device.close(), self.loop)
        try:
            future.result()
        except Exception as e:
            print("Disconnect Error", "Failed to disconnect", str(e))
        self.close_conn.set_callback(None)

    @threaded
    def lock_control(self):
        device_control = lock_control.LockControl()
        device_control.check_status(self.device.turn_on, self.device.turn_off)

        while self.device.isOpen():
            line = self.device.readLine()
            if line == self.device.BUTTON_PRESS:
                is_muted = device_control.toggle()
                if is_muted:
                    self.device.turn_on()
                else:
                    self.device.turn_off()
            time.sleep(0.250)

    def run(self):
        self.app.run()

if __name__ == '__main__':
    rumps.debug_mode(True)
    app = MuteButtonWidget()
    app.run()
