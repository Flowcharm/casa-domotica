import rfid
import time
import cmd
import serial
from enum import Enum

PORT = 'COM7'  # Modify before use
BAUD_RATE = 9600
TIMEOUT = 0.1


class SerialRead(Enum):
    CANCEL_ACTION = 0


screen = serial.Serial(port=PORT, baudrate=BAUD_RATE, timeout=TIMEOUT)


def send_access_denied():
    screen.write(b'\x00')


def send_access_granted():
    screen.write(b'\x01')


def send_edit_cards():
    screen.write(b'\x10')


def read():
    read = screen.readline()
    if (read == SerialRead.CANCEL_ACTION):
        rfid.send_cancel_action()
