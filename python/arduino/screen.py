from . import rfid
from serial import Serial

PORT = 'COM7'  # Modify before use
BAUD_RATE = 9600
TIMEOUT = 0.1


screen = Serial(port=PORT, baudrate=BAUD_RATE, timeout=TIMEOUT)

def send_access_denied():
    print("Screen: Access denied")
    screen.write(b'0')

def send_access_granted():
    print("Screen: Access granted")
    screen.write(b'1')

def send_edit_cards():
    print("Screen: Editing cards")
    screen.write(b'2')

def send_initial_screen():
    print("Screen: Initial screen")
    screen.write(b'3')

def readScreen():
    read = screen.readline()
    if (read == b'\x00'):
        rfid.send_cancel_action()
