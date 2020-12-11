from kivy.utils import platform
from Utilities import run_safely
from Utilities import debug

if platform != "android":
    from glob import glob
    import serial
else:
    from usb4a import usb
    from usbserial4a import serial4a

class USBdevice():
    @run_safely
    def get_usb_devices(self):
        self.usb_device = usb.get_usb_device_list()[0]

    @debug
    @run_safely
    def read_from_device(self):
        msg = self.serial_port.read_all()
        return bytes(msg).decode('utf8')

    def is_open(self):
        return self.serial_port.is_open

    @debug
    @run_safely
    def write_to_device(self, cmd):
        self.serial_port.write(cmd.encode())

    def flush(self):
        self.serial_port.flushInput()

    @run_safely
    def connect_usb(self):
        if platform == "android":
            self.safe_get_usb_devices()
            port = self.usb_device.getDeviceName()
            self.serial_port = serial4a.get_serial_port(
                port, 9600, 8, 'N', 1)
        else:
            port = [port for port in glob('/dev/tty.*') if "usbserial" in port][0] # Only works on Mac
            self.serial_port = serial.Serial(port, 9600, timeout=3)