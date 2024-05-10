from arduino import rfid, screen

board = rfid.board


def send_access_granted():
    board.write(b'3')
    screen.send_access_granted()


def send_access_denied():
    screen.send_access_denied()
