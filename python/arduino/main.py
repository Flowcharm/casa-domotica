from arduino import screen, rfid


def initArduino():
    while True:
        screen.readScreen()
        rfid.readRFID()
