from board import send_cancel_action
import serial

PORT = 'COM3'  # Modify before use
BAUD_RATE = 9600
TIMEOUT = 0.1

screen = serial.Serial(port=PORT, baudrate=BAUD_RATE, timeout=TIMEOUT)


def send_access_denied():
    screen.write(0)


def send_access_granted():
    screen.write(1)


def send_edit_cards():
    screen.write(2)


while True:
    read = screen.readline()
    if (read == 0):
        send_cancel_action()
