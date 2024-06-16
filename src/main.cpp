#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "Audio.h"
#include <SD.h>
#include <Audio.h>
#include <SPI.h>
#include <Base64.h>
#include "Base64.h"

String getLanguageCode(const char* languageName);
String speakText(String text, const char* apiKey, String targetLanguage);
bool saveAudioToSD(const char* base64Audio, const char* filename);

// VARIABLES / CONTRASENYES
const char* ssid = "RedmiNuria";
const char* password = "Patata123";
const char* apiKey = "AIzaSyAK2DlXI83cBLEFxhvFsrUNnMU5m51c_Ms";
const char* apiKey2 = "AIzaSyCz4Pb-7OIi3Gs6LGgJ-XHZ2Xy__hRAeZQ";

// Función para conectar a WiFi
void connectToWiFi() {
    WiFi.begin(ssid, password);
    Serial.print("Conectando a WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("Conectado a la red WiFi");
}

const int chipSelect = 39; // Cambia esto según el pin que uses

bool initSD() {
  SPI.begin(36, 37, 35); // void begin(int8_t sck=-1, int8_t miso=-1, int8_t mosi=-1, int8_t ss=-1);
  if (!SD.begin(chipSelect)) {
    Serial.println("Error al inicializar la tarjeta SD.");
    return false;
  }
  Serial.println("Tarjeta SD inicializada.");
  return true;
}

void setup() {
  Serial.begin(115200);

  // Conectar a WiFi
  connectToWiFi();

  // Inicializar tarjeta SD
  if (!initSD()) {
    return;
  }

  // Texto a convertir en voz
  String text = "Hola, ¿cómo estás?";

  // Idioma de destino
  const char* idioma = "Spanish";
  String targetLanguage = getLanguageCode(idioma);

  // Obtener el audio en Base64 desde Google Cloud Text-to-Speech
  String base64Audio = speakText(text, apiKey, targetLanguage);
  // Verificar si se obtuvo el audio correctamente
  if (base64Audio.length() > 0) {
    // Guardar el audio en la tarjeta SD
    const char* filename = "/audio.WAV";  // Cambia la extensión según el formato del audio
    /*
    if (saveAudioToSD(base64Audio.c_str(), filename)) {
      Serial.println("Archivo de audio guardado correctamente.");
    } else {
      Serial.println("Error al guardar el archivo de audio.");
    }
    */
  } else {
    Serial.println("Error al obtener el audio desde Google Cloud Text-to-Speech.");
  }
}

String getLanguageCode(const char* languageName) {
    HTTPClient http;
    String url = "https://translation.googleapis.com/language/translate/v2/languages?key=";
    url += apiKey2;
    url += "&target=en";  // Para obtener los nombres de idiomas en inglés

    http.begin(url);
    int httpResponseCode = http.GET();


    String languageCode = "";
    if (httpResponseCode == HTTP_CODE_OK) {
        String response = http.getString();
        StaticJsonDocument<4096> doc;
        deserializeJson(doc, response);
        //DynamicJsonDocument doc(4096);
        //deserializeJson(doc, response);

        JsonArray languages = doc["data"]["languages"];
        for (JsonObject language : languages) {
            if (String(language["name"].as<const char*>()).equalsIgnoreCase(languageName)) {
                languageCode = language["language"].as<String>();
                break;
            }
        }
    } else {
        Serial.print("Error en la solicitud: ");
        Serial.println(httpResponseCode);
    }

    http.end();
    return languageCode;
}

String speakText(String text, const char* apiKey, String targetLanguage) {
    HTTPClient http;
    String url = "https://texttospeech.googleapis.com/v1/text:synthesize?key=";
    url += apiKey;

    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    String jsonBody = "{\"input\":{\"text\":\"";
    jsonBody += text;
    jsonBody += "\"},\"voice\":{\"languageCode\":\"";
    jsonBody += targetLanguage;
    jsonBody += "\"},\"audioConfig\":{\"audioEncoding\":\"MP3\"}}";

    int httpResponseCode = http.POST(jsonBody);

    if (httpResponseCode == HTTP_CODE_OK) {
        String response = http.getString();
        // Extraer y retornar el contenido de audio
        int audioStart = response.indexOf("\"audioContent\": \"") + 17;
        int audioEnd = response.indexOf("\"", audioStart);
        String audioContent = response.substring(audioStart, audioEnd);
        Serial.println(response); 
        return audioContent;
    } else {
        Serial.print("Error en la solicitud: ");
        Serial.println(httpResponseCode);
        return "";
    }

    http.end();
}

/*
// Función para decodificar Base64 y guardar el archivo de audio en la tarjeta SD
bool saveAudioToSD(const char* base64Audio, const char* filename) {
  // Calcular la longitud del audio decodificado
  int inputLen = strlen(base64Audio);
  int audioLength = base64_dec_len((char*)base64Audio, inputLen);
  uint8_t* audioData = new uint8_t[audioLength];

  // Decodificar el audio Base64
  int actualLength = base64_decode((char*)audioData, (char*)base64Audio, inputLen);

  if (actualLength != audioLength) {
    Serial.println("Error al decodificar el audio Base64.");
    delete[] audioData;
    return false;
  }

  // Crear y abrir el archivo en la tarjeta SD
  File audioFile = SD.open(filename, FILE_WRITE);
  if (!audioFile) {
    Serial.println("Error al abrir el archivo en la tarjeta SD.");
    delete[] audioData;
    return false;
  }

  // Escribir los datos de audio en el archivo
  audioFile.write(audioData, audioLength);
  audioFile.close();

  Serial.println("Audio guardado exitosamente en la tarjeta SD.");
  delete[] audioData;
  return true;
}
*/

void loop() {
  // Nada que hacer en loop
}