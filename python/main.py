import screen
import rfid

print("Script Started")
while True:
    screen.read()
    rfid.read()

# screen.send_access_granted()
