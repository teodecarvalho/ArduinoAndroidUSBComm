from kivy.utils import platform
from Utilities import try_n_times

if platform != "android":
    from glob import glob
    import serial
else:
    from usb4a import usb
    from usbserial4a import serial4a

class USBdevice():
    def get_usb_devices(self):
        self.usb_device = usb.get_usb_device_list()[0]

    def safe_get_usb_devices(self):
        try_n_times(self.get_usb_devices)

    def read_from_device(self):
        msg = self.serial_port.read_all()
        return bytes(msg).decode('utf8')

    def safe_read_from_device(self):
        return try_n_times(self.read_from_device)

    def is_open(self):
        return self.serial_port.is_open

    def write_to_device(self, cmd):
        self.serial_port.write(cmd.encode())

    def safe_write_to_device(self, **kwargs):
        try_n_times(self.write_to_device, **kwargs)

    def flush(self):
        self.serial_port.flushInput()

    def connect_usb(self):
        if platform == "android":
            self.safe_get_usb_devices()
            port = self.usb_device.getDeviceName()
            self.serial_port = serial4a.get_serial_port(
                port, 9600, 8, 'N', 1)
        else:
            port = [port for port in glob('/dev/tty.*') if "usbserial" in port][0] # Only works on Mac
            self.serial_port = serial.Serial(port, 9600, timeout=3)

    def safe_connect_usb(self):
        try_n_times(self.connect_usb)