#include <Keypad.h>
#include <LiquidCrystal.h>

// Define el tamaño del teclado matricial y la disposición de los pines
const byte ROWS = 4;  // Cuatro filas
const byte COLS = 4;  // Cuatro columnas

char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

byte rowPins[ROWS] = { 39, 41, 43, 45 };
byte colPins[COLS] = { 47, 49, 51, 53 };

// Define el teclado y la pantalla LCD
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
LiquidCrystal lcd(13, 12, 6, 4, 3, 2);

// Define la contraseña predeterminada
char password[] = "1234";

// Variable para almacenar la contraseña ingresada por el usuario
char enteredPassword[5] = "";

void setup() {
  // Inicializa la pantalla LCD
  lcd.begin(16, 2);
  lcd.print("Ingrese clave:");
  Serial.begin(9600);
}

void loop() {
  char key = keypad.getKey();  // Lee la tecla presionada

  // Si se presiona una tecla
  if (key) {
    // Añade la tecla a la contraseña ingresada
    strcat(enteredPassword, &key);
    lcd.setCursor(strlen(enteredPassword) - 1, 1);  // Mueve el cursor a la posición actual
    lcd.print('*');                                 // Muestra "*" en lugar de la tecla ingresada

    // Si la contraseña ingresada tiene 4 dígitos
    if (strlen(enteredPassword) == 4) {
      // Comprueba si la contraseña es correcta
      if (strcmp(enteredPassword, password) == 0) {
        lcd.clear();
        lcd.print("Acceso concedido");
      } else {
        lcd.clear();
        lcd.print("Acceso denegado");
      }
      delay(2000);  // Espera 2 segundos
      lcd.clear();
      lcd.print("Ingrese clave:");
      enteredPassword[0] = '\0';  // Reinicia la contraseña ingresada
    }
  }
}