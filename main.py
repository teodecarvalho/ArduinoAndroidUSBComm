from kivy.app import App
from kivy.uix.boxlayout import BoxLayout
from USBSerial import USBdevice
from time import sleep
from kivy.clock import Clock
from Utilities import try_n_times

class LedWidget(BoxLayout):
    def __init__(self, **kwargs):
        super(LedWidget, self).__init__(**kwargs)
        self.USBdevice = USBdevice()

    def connect_usb(self):
        self.USBdevice.safe_connect_usb()

    def flush(self):
        self.USBdevice.flush()

    def send_cmd(self, cmd):
        self.USBdevice.safe_write_to_device(cmd = cmd)
    
    def rec_msg(self):
        msg = self.USBdevice.safe_read_from_device()
        return msg

    def turn_led_on(self):
        self.send_cmd("<on>")

    def turn_led_off(self):
        self.send_cmd("<of>")
    
    def parse_led_state(self, state):
        return int(state.splitlines()[1])
        
    def get_led_state(self):
        self.flush()
        sleep(.1)
        self.send_cmd("<ls>")
        sleep(.1)
        state = self.rec_msg()
        print(state)
        return self.parse_led_state(state)

    def safe_get_led_state(self):
        return try_n_times(self.get_led_state)

    def update_led_label(self, dt):
        state = self.safe_get_led_state()
        if state:
            self.ids.led_state_label.text = "On"
        else:
            self.ids.led_state_label.text = "Off"

class LedApp(App):
    def build(self):
        return LedWidget()

    def on_start(self):
        self.root.connect_usb()
        Clock.schedule_interval(self.root.update_led_label, 1)

if __name__ == '__main__':
    LedApp().run()