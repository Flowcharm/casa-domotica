import serial
from arduino import screen

PORT = 'COM6'  # Modify before use
BAUD_RATE = 9600
TIMEOUT = 0.1

board = serial.Serial(port=PORT, baudrate=BAUD_RATE, timeout=TIMEOUT)


def send_add_new_card():
    print("RFID: New card")
    board.write(b'\x00')


def send_remove_card():
    print("RFID: Remove card")
    board.write(b'\x01')


def send_cancel_action():
    print("RFID: Cancel RFID action")
    board.write(b'\x10')


def readRFID():
    read = board.readline()
    if (read == b'\x00'):
        print("Access granted!")
        screen.send_access_granted()
    if (read ==  b'\x01'):
        print("Access denied!")
        screen.send_access_denied()
