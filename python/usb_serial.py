import serial
import serial.tools.list_ports
import time
import io
import mic_control
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
    def __init__(self,port, baudrate = 9600):
        self.BUTTON_PRESS = "0";
        self.port = port;
        self.baudRate = baudrate
        self.connection = self.connect(self.port, self.baudRate)

        #TODO: Set the timer to read the line

    def turn_on(self):
        self.connection.write(b'1') # write a string
        

    def turn_off(self):
        self.connection.write(b'0') # write a string
        

    def close(ser):
        ser.close() # close port
            

    def connect_with_context(port, onRead, baudRate = 9600, ):
        with serial.Serial(port, baudRate) as ser:
            line = ser.readline() 
            #onRead(line);
            if(line == '0'):
                print("turn of mic")

    def connect(self,port, onRead, baudRate = 9600, ):
        ser = serial.Serial(port, baudRate)
        return ser;


    def readLine(self):
        line = self.connection.readline() 
        return line.decode("utf8").strip()
      

if __name__ == "__main__":
    ports = get_ports()
    for port in ports:
        print(port.device)
    mic_device = mic_control.MicControl()
    serial_device = UsbSerial('/dev/cu.usbmodem14301');
    while True:
        line = serial_device.readLine()
        print(line)
        if line == serial_device.BUTTON_PRESS:
            is_muted = mic_device.toggle()
            if(is_muted):
                serial_device.turn_on();
            else:
                serial_device.turn_off();

        time.sleep(1)

