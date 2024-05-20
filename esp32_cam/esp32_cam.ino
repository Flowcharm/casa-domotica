#include "esp_timer.h"
#include "esp_camera.h"

#include <WiFi.h>
#include "img_converters.h"
#include "fb_gfx.h"
#include <ArduinoWebsockets.h>

#include "config.h"

char* incomingData = NULL; 

using namespace websockets;
WebsocketsClient client;

enum state {
  WAITING_FOR_WIFI,
  WAITING_FOR_SERVER,
  ACCEPTED
};

// TODO: Responder por Serial el estado de la conexión para manejarlo en la raspberry
enum WiFiState {
  DISCONNECTED,
  CONNECTING,
  CONNECTED
};

int connectionState = WAITING_FOR_WIFI;
bool mustSendImage = false;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println("Inicializando...");

  initCamera();

#if defined(pinLed)
  // TODO
#endif

#if defined (DEFAULT_SSID) && defined(DEFAULT_PASSWORD)
  connectToWifi(DEFAULT_SSID, DEFAULT_PASSWORD);
#endif
}

void loop() {
  if(connectionState == WAITING_FOR_WIFI) {
    connectToWifi();
  } 

  if (connectionState == WAITING_FOR_SERVER) {
    initClient();
  }

  if(connectionState == CONNECTED) {
    if (client.available()) {
      client.poll();
      if (mustSendImage) {
        sendImage();
      } 
    } else {
      connectionState = WAITING_FOR_SERVER;
      mustSendImage = false;
    }
  }
  delay(1);
}

void onMessageSocket(WebsocketsMessage message) {
  Serial.print("Mensaje: ");
  Serial.println(message.data());

  if(message.data().startsWith("[STREAM]")){
    incomingData = (char*)message.data().c_str();
    incomingData += 8;
    Serial.print("Comando de stream recibido: ");
    Serial.println(incomingData);
    if (strcmp(incomingData, "[START]") == 0) {
      Serial.println("Iniciando stream");
      mustSendImage = true;
    } else if (strcmp(incomingData, "[PAUSE]") == 0) {
      Serial.println("Stream pausado");
      mustSendImage = false;
    }
  } else if(message.data().startsWith("[COMMAND]")){
    incomingData = (char*)message.data().c_str();
    incomingData += 9;
    Serial.print("Comando recibido: ");
    Serial.println(incomingData);
    // TODO: Implementar comandos, como cambiar la resolución de la cámara o encender el led
  }
}
void onEventSocket(WebsocketsEvent event, String data) {
  if (event == WebsocketsEvent::ConnectionClosed) {
    Serial.println("Desconectado del servidor");
    mustSendImage = false;
    connectionState = WAITING_FOR_SERVER;
  }
}
esp_err_t initClient() {
  
#if defined (DEFAULT_SOCKET_HOST)
  const char* socketHost = DEFAULT_SOCKET_HOST;
#else
  // TODO: Se debe de recibir la ip del servidor por serial si no hay un host default
  const char* socketHost = "0.0.0.0";
#endif
  Serial.print("Intentando conectar el cliente al servidor: ");
  Serial.print(socketHost);


  client.onMessage(onMessageSocket);
  client.onEvent(onEventSocket);

  bool isConnected = false;

  
  while(!isConnected) {
    Serial.print(".");
    isConnected = client.connect(socketHost, SOCKET_PORT, "/camera");
    delay(1000);
  }

  Serial.println("");
  Serial.println("WS OK");
  connectionState = CONNECTED;
  return ESP_OK;
}

esp_err_t sendImage() {
  camera_fb_t *frame_buffer = esp_camera_fb_get();

  if (!frame_buffer) {
    Serial.println("Camera capture failed");
    esp_camera_fb_return(frame_buffer);
    return ESP_FAIL;
  }

  if (frame_buffer->format != PIXFORMAT_JPEG) {
    Serial.println("Non-JPEG data not implemented");
    esp_camera_fb_return(frame_buffer);
    return ESP_FAIL;
  }

  bool sent = client.sendBinary((const char *)frame_buffer->buf, frame_buffer->len);
  esp_camera_fb_return(frame_buffer);

  if (!sent) {
    Serial.println("Error al enviar la imagen");
    return ESP_FAIL;
  }
  Serial.println("Imagen enviada correctamente");
  return ESP_OK;
}

void onEventWifi(WiFiEvent_t event, WiFiEventInfo_t info) {
  if (event == SYSTEM_EVENT_STA_DISCONNECTED) {
    Serial.println("Desconectado de la red WiFi");
    Serial.write(DISCONNECTED);
    WiFi.disconnect();
    connectionState = WAITING_FOR_WIFI;
  }
}
esp_err_t connectToWifi(const char* ssid, const char* password) {
  Serial.print("Conectando a la red WiFi..");

  WiFi.begin(ssid, password);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(false);

  int retries = 0;
  Serial.write(CONNECTING);
  while (WiFi.status() != WL_CONNECTED && retries < 50) {
    delay(500);
    Serial.print(".");
    retries++;
  }
  Serial.println("");

  if (retries >= 50) {
    Serial.println("Fallo al conectar a la red WiFi:");
    Serial.print("SSID: ");
    Serial.println(ssid);

    WiFi.disconnect();

    return ESP_FAIL;
  }

  WiFi.setAutoReconnect(true);
  WiFi.onEvent(onEventWifi);

  Serial.println("Conectado a la red WiFi:");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  connectionState = WAITING_FOR_SERVER;
  Serial.write(CONNECTED);
  return ESP_OK;
}
esp_err_t connectToWifi() {
  // TODO
  if (Serial.available() > 0) {
    // Formato: ssid:_:password
    String ssidAndPassword = Serial.readStringUntil('\n');
    ssidAndPassword.trim();
    int separatorIndex = ssidAndPassword.indexOf(":_:");

    String ssid = ssidAndPassword.substring(0, separatorIndex);
    String password = ssidAndPassword.substring(separatorIndex + 3);
    return connectToWifi(ssid.c_str(), password.c_str());
  } else {
    return ESP_FAIL;
  }
}
