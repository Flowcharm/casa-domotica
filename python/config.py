import os
from twilio.rest import Client
from dotenv import load_dotenv

load_dotenv()

phone_number_to = os.getenv("PHONE_NUMBER_TO")
phone_number_from = os.getenv("PHONE_NUMBER_FROM")
account_sid = os.getenv("TWILIO_ACCOUNT_SID")
auth_token = os.getenv("TWILIO_AUTH_TOKEN")

url = ""
camHost = ""
camPort = "8000"
grantUrl = f"{url}/grant-access"
denyUrl = f"{url}/deny-access"

client = Client(account_sid, auth_token)
