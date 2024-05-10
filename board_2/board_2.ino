#include <SPI.h>
#include <MFRC522.h>
#include <Streaming.h>
#include <Vector.h>
#include <Servo.h>

#define POTENCIOMETER_PIN 44
#define OPEN_DOOR_TIME 2000  // ms

#define BUZZER_PIN 45
#define BUZZER_DURATION 500  // ms
#define BUZZER_TONE 3000     // Khz

#define LED_DELAY 500
#define LED_GREEN_PIN 47
#define LED_RED_PIN 46
#define LED_BLUE_PIN 48

#define TEMPERATURE_SENSOR_PIN A0

#define ASCII_0 48
#define ASCII_1 49
#define ASCII_2 50
#define ASCII_3 51

#define MAX_REGISTERED_CARDS 10

#define RST_PIN 9                  // Reset pin for RC522
#define SS_PIN 53                  // Slave select pin for RC522
#define LED_PIN_VALID 23           // Pin connected to the valid access LED
#define LED_PIN_ERROR 30           // Pin connected to the error LED
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

String storage_array[MAX_REGISTERED_CARDS];

Vector<String> registeredCards;

enum state {
  Add_card,
  Remove_card,
  Reading
};

state currentState = Reading;

int incomingData = -1;

Servo servo;

enum digitalWrite {
  Access_granted,
  Access_denied,
  Card_edited,
  Open_door
};

void setup() {
  Serial.begin(9600);  // Start serial communication for debugging
  SPI.begin();         // Initialize SPI bus
  mfrc522.PCD_Init();  // Initialize the MFRC522
  registeredCards.setStorage(storage_array);
  Serial.println("Place a card to verify access permissions");

  servo.attach(POTENCIOMETER_PIN);

  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Default access granted card
  registeredCards.push_back("F3BF3011");

  closeDoor();
}

void loop() {
  checkCardActions();
  rfidModule();
}


void rfidModule() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    // Create sound
    tone(BUZZER_PIN, BUZZER_TONE);
    delay(BUZZER_DURATION);
    noTone(BUZZER_PIN);

    // Extract the UID from the card
    String uidStr = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      uidStr += (mfrc522.uid.uidByte[i] < 0x10 ? "0" : "") + String(mfrc522.uid.uidByte[i], HEX);
    }
    uidStr.toUpperCase();  // Convert UID to uppercase to match the UID strings

    Serial.print("Card UID: ");
    Serial.println(uidStr);

    switch (currentState) {
      case Reading:
        Serial.println("Card read");
        // Check if the UID matches one of the allowed UIDs
        if (isCardRegistered(uidStr)) {
          acceptedCardLed();
          Serial.println("Card is valid and has permissions");

          openAndCloseDoor();

          Serial.write(Access_granted);
        } else {
          Serial.println("Card does not have permissions");
          deniedCardLed();
          Serial.write(Access_denied);
        }
        break;
      case Add_card:
        actionCardLed();
        Serial.println("New card added");
        registeredCards.push_back(uidStr);
        currentState = Reading;
        Serial.write(Card_edited);
        break;
      case Remove_card:
        actionCardLed();
        Serial.println("Card deleted");
        removeCard(uidStr);
        currentState = Reading;
        Serial.write(Card_edited);
        break;
    }


    mfrc522.PICC_HaltA();  // Halt PICC
  }
}

void openAndCloseDoor() {
  openDoor();
  delay(OPEN_DOOR_TIME);
  closeDoor();
}

void closeDoor() {
  servo.write(160);
}

void openDoor() {
  servo.write(60);
}

void acceptedCardLed() {
  digitalWrite(LED_RED_PIN, 0);
  digitalWrite(LED_GREEN_PIN, 255);
  digitalWrite(LED_BLUE_PIN, 0);

  delay(LED_DELAY);

  resetLed();
}

void deniedCardLed() {
  digitalWrite(LED_RED_PIN, 255);
  digitalWrite(LED_GREEN_PIN, 0);
  digitalWrite(LED_BLUE_PIN, 0);

  delay(LED_DELAY);

  resetLed();
}

void actionCardLed() {
  digitalWrite(LED_RED_PIN, 0);
  digitalWrite(LED_GREEN_PIN, 0);
  digitalWrite(LED_BLUE_PIN, 255);

  delay(LED_DELAY);

  resetLed();
}

void resetLed() {
  digitalWrite(LED_RED_PIN, 0);
  digitalWrite(LED_GREEN_PIN, 0);
  digitalWrite(LED_BLUE_PIN, 0);
}

void removeCard(String uid) {
  for (int i = 0; i < registeredCards.size(); i++) {
    if (registeredCards[i] == uid) {
      registeredCards.remove(i);
    }
  }
}

void checkCardActions() {
  if (Serial.available() > 0) {
    incomingData = Serial.read();

    Serial.print("Mensaje capturado: ");
    Serial.println(incomingData);

    // 0: Add new card
    // 1: Remove card
    // 2: Cancel action
    // 3: Open door
    switch (incomingData) {
      case 0:
      case ASCII_0:
        Serial.println("Add card");
        currentState = Add_card;
        break;
      case 1:
      case ASCII_1:
        Serial.println("Remove card");
        currentState = Remove_card;
        break;
      case 2:
      case ASCII_2:
        Serial.println("Cancel action");
        currentState = Reading;
        break;
      case 3:
      case ASCII_3:
        Serial.println("Door opened");
        openAndCloseDoor();
        break;
    }
  }
}

bool isCardRegistered(String id) {
  bool isRegistered = false;

  for (String registeredId : registeredCards) {
    if (registeredId == id) {
      isRegistered = true;
    }
  }

  return isRegistered;
}