import threading
import time
from config import url, isCallAvailable, camUrl, grantUrl, denyUrl
from flask import Flask, render_template
from arduino import rfid, door, main

app = Flask(__name__)


@app.route("/")
def index():
    return render_template("index.html", url)


@app.route("/cam")
def cam():
    return render_template("cam.html", isCallAvailable, camUrl, grantUrl, denyUrl)


@app.route("/add-card")
def add_card():
    rfid.send_add_new_card()
    return "new card"


@app.route("/remove-card")
def remove_card():
    rfid.send_remove_card()
    return "card removed"


@app.route("/grant-access")
def grant_access():
    door.send_access_granted()
    return "Access granted"


@app.route("/deny-access")
def deny_access():
    door.send_access_denied()
    return "Access denied"


def web():
    app.run(debug=False, use_reloader=False, host='0.0.0.0', port=5000)
    # app.run()


if __name__ == "__main__":
    threading.Thread(target=web, daemon=True).start()
    threading.Thread(target=main.initArduino, daemon=True).start()
    while True:
        time.sleep(1)
