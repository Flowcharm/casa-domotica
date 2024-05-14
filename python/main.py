import os
import threading
import time
from twilio.rest import Client
from flask import Flask, render_template
from arduino import rfid, door, main
from dotenv import load_dotenv

load_dotenv()

phone_number_to = os.getenv("PHONE_NUMBER_TO")
phone_number_from = os.getenv("PHONE_NUMBER_FROM")
account_sid = os.getenv("TWILIO_ACCOUNT_SID")
auth_token = os.getenv("TWILIO_AUTH_TOKEN")

client = Client(account_sid, auth_token)

app = Flask(__name__)

url = ""
camUrl = ""
isCallAvailable = False
grantUrl = f"{camUrl}/grant-access"
denyUrl = f"{camUrl}/deny-access"

message = client.messages.create(
    from_=phone_number_from,
    body=f"¡Hola! te están llamando al telefonillo puedes responder la llamada desde este enlace {url}/cam",
    to=phone_number_to
)


@app.route("/")
def index():
    return render_template("index.html", url)


@app.route("/cam")
def index():
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
