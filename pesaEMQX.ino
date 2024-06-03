#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>       
#include <RTClib.h>     
#include "HX711.h"		

const char* ssid = "DiositoPlus"; 
const char* password = "P3P3_xdm"; 
const char* mqtt_broker = "143.198.135.231";
const char* mqtt_topic_reloj = "Reloj"; // Nombre del tópico para publicar la fecha y hora
const char* mqtt_topic_peso = "Peso"; // Nombre del tópico para publicar el peso
const int mqtt_port = 1883;
const int scl_pin = 21;
const int sda_pin = 22;
#define DT 2			
#define SCK 4			

WiFiClient espClient;
PubSubClient client(espClient);

RTC_DS3231 rtc;			
HX711 celda;

void setup() {
  Serial.begin(9600);   
  Wire.begin(sda_pin, scl_pin);
  rtc.begin();
  celda.begin(DT, SCK);	// Inicializa el objeto HX711
  celda.set_scale(483.f);	// establece el factor de escala obtenido del primer programa
  celda.tare();	
  
  if (!rtc.begin()) {				
    Serial.println("Modulo RTC no encontrado !");
    while (1);
  }

  WiFi.begin(ssid, password); 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Conectando a WiFi...");
  }
  Serial.println("Conectado a WiFi");
  
  client.setServer(mqtt_broker, mqtt_port); 
}

void loop() {
  if (!client.connected()) {
    reconnect(); 
  }
  client.loop(); 

  DateTime now = rtc.now();
  char fechaHora[25];
  sprintf(fechaHora, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
  client.publish(mqtt_topic_reloj, fechaHora, true); // Publicar la fecha y hora al tópico "Reloj"

  float peso = celda.get_units(10); // Obtener el peso promedio de 10 lecturas
  char pesoStr[15];
  sprintf(pesoStr, "%.2f", peso); // Convertir el peso a cadena
  client.publish(mqtt_topic_peso, pesoStr, true); // Publicar el peso al tópico "Peso"

  delay(1000); // Demora para no saturar el broker, ajustar según necesidad
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("conectado");
    } else {
      Serial.print("falló, rc=");
      Serial.print(client.state());
      Serial.println(" intenta en 5 segundos");
      delay(5000);
    }
  }
}
