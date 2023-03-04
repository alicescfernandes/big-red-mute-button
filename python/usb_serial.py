import serial
import serial.tools.list_ports
import time
import sys
import getopt
from controls import mic_control, lock_control

#with serial.Serial('/dev/ttyS1', 19200, timeout=1) as ser:
#    ser.write(b'hello')     # write a string
#    ser.close()             # close port
#    x = ser.read()          # read one byte
#    s = ser.read(10)        # read up to ten bytes (timeout)
#    line = ser.readline()   # read a '\n' terminated line


def get_ports():
    ports = serial.tools.list_ports.comports()
    return ports


class UsbSerial:
    def __init__(self, baudrate = 9600):
        self.BUTTON_PRESS = "0";
        self.baudRate = baudrate
        self.connection = None
        # self.connection = self.connect(self.port, self.baudRate)

        #TODO: Set the timer to read the line

    def turn_on(self):
        self.connection.write(b'1') # write a string

    def turn_off(self):
        if(self.connection is not None):
            self.connection.write(b'0') # write a string

    def close(self):
        if(self.connection is not None):
            self.connection.close() # close port
            self.connection = None

    def isOpen(self):
        if(self.connection):
            return self.connection.isOpen()
        return False

    def connect_with_context(self,port, onRead):
        self.port = port
        with serial.Serial(port, self.baudRate) as ser:
            line = ser.readline() 
            onRead(line);
            if(line == '0'):
                print("turn of mic")

    def connect(self,port):
        self.port = port
        self.connection = serial.Serial(port, self.baudRate)
        self.connection
        return self.connection;

    def readLine(self):
        if(self.connection is None):
            return;
        line = self.connection.readline() 
        return line.decode("utf8").strip()

    
    def get_ports():
        ports = serial.tools.list_ports.comports()
        return ports



if __name__ == "__main__":
    serial_device = UsbSerial()
    device_control = mic_control.MicControl()
    
    args = sys.argv[1:]
    if("--lock" in args):
        device_control = lock_control.LockControl()

    if("--list" in args):
        ports = serial_device.get_ports()
        print(ports)
        for port in ports:
            print(port.device, port.description)    

    elif("--port" in args):
        index = args.index("--port");
        serial_device.connect(args[index+1])
        device_control.check_status(serial_device.turn_on, serial_device.turn_off)
        while serial_device.isOpen():
            line = serial_device.readLine()
            if line == serial_device.BUTTON_PRESS:
                is_muted = device_control.toggle()
                if(is_muted):
                    serial_device.turn_on();
                else:
                    serial_device.turn_off();

            time.sleep(0.250)

