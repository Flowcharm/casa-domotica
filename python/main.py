import threading
import time
from config import url, camHost, camPort, grantUrl, denyUrl
from flask import Flask, render_template
from arduino import rfid, door, main, screen
from esp32.StreamServer import StreamServer

app = Flask(__name__)

isCallAvailable = False


@app.route("/")
def index():
    return render_template("index.html", url=url)


@app.route("/cam")
def cam():
    global isCallAvailable
    if isCallAvailable:
        screen.send_call_in_progress()
    return render_template("cam.html", isCallAvailable=isCallAvailable, camHost=camHost, camPort=camPort, grantUrl=grantUrl, denyUrl=denyUrl)


@app.route("/add-card")
def add_card():
    global isCallAvailable
    isCallAvailable = False
    rfid.send_add_new_card()
    return "new card"


@app.route("/remove-card")
def remove_card():
    global isCallAvailable
    isCallAvailable = False
    rfid.send_remove_card()
    return "card removed"


@app.route("/grant-access")
def grant_access():
    global isCallAvailable
    isCallAvailable = False
    door.send_access_granted()
    return "Access granted"


@app.route("/deny-access")
def deny_access():
    global isCallAvailable
    isCallAvailable = False
    door.send_access_denied()
    return "Access denied"

@app.route("/door-webhook")
def door_webhook():
    global isCallAvailable
    isCallAvailable = True
    return "Webhook"

def web():
    app.run(debug=False, host='0.0.0.0', port=5000)
    # app.run()


def socket():
    streamServer = StreamServer(host=camHost, port=camPort)
    streamServer.run()

if __name__ == "__main__":
    web = threading.Thread(target=web)
    arduino = threading.Thread(target=main.initArduino)
    stream = threading.Thread(target=socket)

    web.start()
    arduino.start()
    stream.start()

    web.join()
    arduino.join()
    stream.join()
