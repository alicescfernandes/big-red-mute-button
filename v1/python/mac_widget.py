import rumps
import usb_serial
from utils import threaded 
from controls import lock_control
import time

class MuteButtonWidget(object):
    def __init__(self):
        self.config = {
            "app_name": "Mute Widget",
        }
        self.devices_list = {}
        
        self.app = rumps.App(self.config["app_name"])
        self.set_up_menu()
        
        self.timer = rumps.Timer(self.on_tick, 5)
        
        self.list_devices = rumps.MenuItem(title="Devices List", callback=self.on_tick)
        self.timer.start()
        self.serial_device = usb_serial.UsbSerial()
        self.close_conn = rumps.MenuItem(title="Disconnect Device")
        self.app.menu = [self.list_devices, self.close_conn]

    def set_up_menu(self):
        self.app.title = "🔴"
    
    def on_tick(self, sender):
        devices = usb_serial.get_ports()

        self.devices_list = {}

        if(self.list_devices.items().__len__()):
            self.list_devices.clear()

        for device in devices:
            list_id = device.description.replace(" ", "_").lower()
            self.devices_list[list_id] = device
            device_m = rumps.MenuItem(title=device.description, callback=self.connect_to_device)
            self.list_devices.add(device_m)


    def connect_to_device(self, sender):
        sender_desc = sender.title;
        list_id = sender.title.replace(" ", "_").lower()

        device = self.devices_list[list_id]
        self.serial_device.connect(device.device)
        self.close_conn.set_callback(self.disconnect_device)
        self.lock_control()
        


    def disconnect_device(self, sender):
        self.serial_device.close()
        self.close_conn.set_callback(None)

    @threaded
    def lock_control(self):
        device_control = lock_control.LockControl()
        device_control.check_status(self.serial_device.turn_on, self.serial_device.turn_off)
        while self.serial_device.isOpen():
            line = self.serial_device.readLine()
            if line == self.serial_device.BUTTON_PRESS:
                is_muted = device_control.toggle()
                if(is_muted):
                    self.serial_device.turn_on();
                else:
                    self.serial_device.turn_off();

            time.sleep(0.250)

    def run(self):
        self.app.run()

if __name__ == '__main__':
    rumps.debug_mode(True)
    app = MuteButtonWidget()
    app.run()