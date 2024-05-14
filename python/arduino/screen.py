from config import client, phone_number_from, phone_number_to, url
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

def send_call_in_progress():
    print("Call in progress")
    screen.write(b'4')

def sendMessage():
    client.messages.create(
        from_=phone_number_from,
        body=f"¡Hola! te están llamando al telefonillo puedes responder la llamada desde este enlace {url}/cam",
        to=phone_number_to
    )

def readScreen():
    read = screen.readline()
    if (read == b'\x00'):
        rfid.send_cancel_action()
    elif (read == b'\x01'):
        sendMessage()
        send_call_in_progress()