import threading
import time
from flask import Flask, render_template
# from arduino import rfid, main

app = Flask(__name__)


@app.route("/")
def index():
    return render_template("index.html")


@app.route("/add-card")
def add_card():
    # rfid.send_add_new_card()
    return "new card"


@app.route("/remove-card")
def remove_card():
    # rfid.send_remove_card()
    return "card removed"


def web():
    # app.run(debug=True, use_reloader=False, host='0.0.0.0', port=5000)
    app.run()


if __name__ == "__main__":
    threading.Thread(target=web, daemon=True).start()
    # threading.Thread(target=main.initArduino, daemon=True).start()
    while True:
        time.sleep(1)
