#include <Adafruit_GFX.h>
#include <Adafruit_TFTLCD.h>
#include <TouchScreen.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMono9pt7b.h>

#define YP A1
#define XM A2
#define YM 7
#define XP 6

#define SCREEN_DELAY 250 // ms

#define ASCII_0 48
#define ASCII_1 49
#define ASCII_2 50
#define ASCII_3 51
#define ASCII_4 52

#define LCD_RESET A4

// Colors
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

#define CALL_TIMEOUT 1500

enum state {
  Initial,
  Calling,
  Permission_granted,
  Permission_denied,
  Call_in_progress,
  Card_edit
};

enum serialWrite {
  Cancel_action,
  Calling_action
};

state currentState = Initial;
Adafruit_TFTLCD tft;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

const int16_t SCREEN_WIDTH = tft.width();
const int16_t SCREEN_HEIGHT = tft.height();

int incomingData = -1;

void setup() {
  Serial.begin(9600);

  tft.reset();

  uint16_t identifier = tft.readID();
  tft.begin(identifier);
  initialScreen();
}

void loop() {
  checkAccessControl();
  if (ts.isTouching() && currentState == Initial) {
    callingScreen();

    // Create a delay to prevent call spam
    delay(CALL_TIMEOUT);
  }
}

void initialScreen() {
  currentState = Initial;
  configureTouchScreenPins();
  int16_t mid_screen_y = SCREEN_HEIGHT / 2;
  tft.fillScreen(WHITE);
  tft.setFont(&FreeMonoBold12pt7b);
  tft.setCursor(50, mid_screen_y);
  tft.setTextSize(1);
  tft.setTextColor(BLACK);
  tft.println("Bienvenido");

  tft.setFont(&FreeMono9pt7b);
  tft.setCursor(20, SCREEN_HEIGHT - 10);
  tft.setTextSize(1);
  tft.setTextColor(BLACK);
  tft.println("Pulsar para llamar");
}

void callingScreen() {
  currentState = Calling;

  configureTouchScreenPins();
  int16_t mid_screen_y = SCREEN_HEIGHT / 2;
  int ANIMATION_FRAMES = 4;
  int loopCount = 0;
  tft.fillScreen(WHITE);
  Serial.write(Calling_action);

  tft.setFont(&FreeMonoBold12pt7b);
  tft.setCursor(45, mid_screen_y);
  tft.setTextSize(1);
  tft.setTextColor(BLACK);
  tft.println("Llamando");

  tft.setFont(&FreeMono9pt7b);
  tft.setCursor(10, SCREEN_HEIGHT - 10);
  tft.setTextSize(1);
  tft.setTextColor(BLACK);
  tft.println("Pulsar para cancelar");
  delay(SCREEN_DELAY);
  while (currentState == Calling) {
    tft.setFont(&FreeMonoBold12pt7b);
    tft.setCursor(45, mid_screen_y);
    tft.setTextSize(1);
    tft.setTextColor(BLACK);
    if (loopCount == 0) {
      tft.println("Llamando");
      tft.fillRect(160, 150, 50, 15, WHITE);
    } else if (loopCount == 1) {
      tft.println("Llamando.");
    } else if (loopCount == 2) {
      tft.println("Llamando..");
    } else if (loopCount == 3) {
      tft.println("Llamando...");
    }

    loopCount = (loopCount + 1) % ANIMATION_FRAMES;

    if (ts.isTouching()) {
      Serial.println("Saliendo de la peticion de llamada...");
      initialScreen();
    } else {
      configureTouchScreenPins();
      delay(SCREEN_DELAY);
    }
    checkAccessControl();
  }
}

void accessGrantedScreen() {
  currentState = Permission_granted;

  int16_t margin_top = 20;
  int16_t margin_top_check = 20;
  int16_t mid_screen_x = SCREEN_WIDTH / 2;
  int16_t mid_screen_y = (SCREEN_HEIGHT / 2) + margin_top;

  configureTouchScreenPins();

  tft.fillScreen(WHITE);
  tft.drawCircle(mid_screen_x, mid_screen_y, 55, GREEN);

  tft.drawLine(mid_screen_x, mid_screen_y + margin_top_check, 90, 150 + margin_top_check, GREEN);
  tft.drawLine(mid_screen_x, mid_screen_y + margin_top_check, 180, 100 + margin_top_check, GREEN);

  tft.setFont(&FreeMonoBold12pt7b);
  tft.setCursor(65, mid_screen_x - margin_top);
  tft.setTextSize(1);
  tft.setTextColor(BLACK);
  tft.println("Aprobado");
}

void accessDeniedScreen() {
  currentState = Permission_denied;

  int16_t margin_top = 20;
  int16_t margin_top_cross = 30;
  int16_t cross_gap = 30;
  int16_t mid_screen_x = SCREEN_WIDTH / 2;
  int16_t mid_screen_y = (SCREEN_HEIGHT / 2) + margin_top;

  configureTouchScreenPins();

  tft.fillScreen(WHITE);
  tft.drawCircle(mid_screen_x, mid_screen_y, 55, RED);

  tft.drawLine(mid_screen_x + cross_gap, mid_screen_y + margin_top_cross, 90, 150, RED);
  tft.drawLine(mid_screen_x - cross_gap, mid_screen_y + margin_top_cross, 150, 150, RED);

  tft.setFont(&FreeMonoBold12pt7b);
  tft.setCursor(65, mid_screen_x - margin_top);
  tft.setTextSize(1);
  tft.setTextColor(BLACK);
  tft.println("Denegado");
}

void callInProgressScreen() {
  currentState = Call_in_progress;

  configureTouchScreenPins();
  int16_t mid_screen_y = SCREEN_HEIGHT / 2;
  int ANIMATION_FRAMES = 4;
  int loopCount = 0;
  tft.fillScreen(WHITE);
  Serial.println("¡Llamada en curso!");

  tft.setFont(&FreeMonoBold12pt7b);
  tft.setCursor(45, mid_screen_y);
  tft.setTextSize(1);
  tft.setTextColor(BLACK);
  tft.println("En curso");

  tft.setFont(&FreeMono9pt7b);
  tft.setCursor(20, SCREEN_HEIGHT - 10);
  tft.setTextSize(1);
  tft.setTextColor(BLACK);
  tft.println("Pulsar para colgar");
  delay(SCREEN_DELAY);
  while (currentState == Call_in_progress) {
    tft.setFont(&FreeMonoBold12pt7b);
    tft.setCursor(45, mid_screen_y);
    tft.setTextSize(1);
    tft.setTextColor(BLACK);
    if (loopCount == 0) {
      tft.println("En curso");
      tft.fillRect(160, 150, 50, 15, WHITE);
    } else if (loopCount == 1) {
      tft.println("En curso.");
    } else if (loopCount == 2) {
      tft.println("En curso..");
    } else if (loopCount == 3) {
      tft.println("En curso...");
    }

    loopCount = (loopCount + 1) % ANIMATION_FRAMES;

    if (ts.isTouching()) {
      Serial.println("Saliendo de la llamada...");
      initialScreen();
    } else {
      configureTouchScreenPins();
      delay(SCREEN_DELAY);
    }

    checkAccessControl();
  }
}

void cardEditScreen() {
  configureTouchScreenPins();
  currentState = Card_edit;

  int16_t mid_screen_x = SCREEN_WIDTH / 2;
  int16_t mid_screen_y = SCREEN_HEIGHT / 2;

  tft.fillScreen(WHITE);
  tft.setFont(&FreeMonoBold12pt7b);
  tft.setCursor(80, mid_screen_y);
  tft.setTextSize(1);
  tft.setTextColor(BLACK);
  tft.println("Acerca");

  tft.setFont(&FreeMonoBold12pt7b);
  tft.setCursor(50, mid_screen_y + 25);
  tft.setTextSize(1);
  tft.setTextColor(BLACK);
  tft.println("la tarjeta");

  tft.setFont(&FreeMono9pt7b);
  tft.setCursor(10, SCREEN_HEIGHT - 10);
  tft.setTextSize(1);
  tft.setTextColor(BLACK);
  tft.println("Pulsar para cancelar");

  while (currentState == Card_edit) {
    checkAccessControl();
    if (ts.isTouching()) {
      Serial.println("Cancelando accion...");
      Serial.write(Cancel_action);
      initialScreen();
    }
  }
}

void checkAccessControl() {
  int16_t screen_delay = 2000;  // ms

  if (Serial.available() > 0) {
    incomingData = Serial.read();

    Serial.print("Mensaje capturado: ");
    Serial.println(incomingData);

    // 0: acceso denegado
    // 1: acceso concedido
    // 2: modificar tarjetas
    // 3: pantalla inicial
    // 4: llamada en curso
    switch (incomingData) {
      case 0:
      case ASCII_0:
        Serial.println("Acceso denegado");

        accessDeniedScreen();

        delay(screen_delay);

        initialScreen();
        break;
      case 1:
      case ASCII_1:
        Serial.println("Acceso concedido");

        accessGrantedScreen();

        delay(screen_delay);

        initialScreen();
        break;
      case 2:
      case ASCII_2:
        Serial.println("Modificando tarjetas");

        cardEditScreen();

        delay(screen_delay);
        break;
      case 3:
      case ASCII_3:
        Serial.println("Pantalla principal");

        initialScreen();
        break;
      case 4:
      case ASCII_4:
        Serial.println("Llamada en curso");

        callInProgressScreen();
        break;
    }
  }
}

void configureTouchScreenPins() {
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
}
