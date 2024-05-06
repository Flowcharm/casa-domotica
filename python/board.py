import serial

PORT = 'COM3'  # Modify before use
BAUD_RATE = 9600
TIMEOUT = 0.1

board = serial.Serial(port=PORT, baudrate=BAUD_RATE, timeout=TIMEOUT)


def send_add_new_card():
    board.write(0)


def send_remove_card():
    board.write(1)


def send_cancel_action():
    board.write(2)
