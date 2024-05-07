import serial
import screen
from enum import Enum


# class SerialRead(Enum):
#     ACCESS_GRANTED = 0,
#     ACCESS_DENIED = b'\x01'


PORT = 'COM6'  # Modify before use
BAUD_RATE = 9600
TIMEOUT = 0.1

board = serial.Serial(port=PORT, baudrate=BAUD_RATE, timeout=TIMEOUT)


def send_add_new_card():
    board.write(b'\x00')


def send_remove_card():
    board.write(b'\x01')


def send_cancel_action():
    board.write(b'\x10')


def read():
    read = board.readline()
    if (read == b'\x00'):
        print("Access granted!")
        screen.send_access_granted()
    if (read ==  b'\x01'):
        print("Access denied!")
        screen.send_access_denied()
