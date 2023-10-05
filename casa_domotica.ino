#include <DHTesp.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include "Adafruit_SGP30.h"

// definisco i pin del dht: tipo e pin in cui è collegato
#define LIGHT 14 // led bianco
#define DHTPIN 27 
#define FINESTRA 26 // servomotore
#define DHTTYPE DHTesp::DHT22
#define VENTOLA 12 // rele
#define SCL 33 // per sgp30
#define SDA 32 // per sgp30

const char* ssid1 = "FileHubPlus-2474";
const char* password1 = "11111111";
const char* ssid2 = "Ziro WiFi";
const char* password2 = "Mirko.2005";
const char* mqtt_server = "broker.hivemq.com";

Adafruit_SGP30 sgp;
DHTesp dht;
Servo servo;
WiFiClient net;
PubSubClient client(net);

bool statoLed; // stato del led per controllo incrociato
uint8_t statoVentilazionne = 0;
uint8_t statoFinestra = 0;
uint8_t statoLuce = 0;
uint8_t statoVentola = 0; // stato per la ventola per il controllo incrociato e il sync con la dashboard
uint8_t status = 0; // stato per fare il controllo incrociato con la dashboard
const char* topicLuceSub = "casa/luce/dash"; // topic per la luce (dashboard --> scheda)
const char* topicHum = "casa/com/hum"; // topic per mandare alla dashboard l'umidità rilevata
const char* topicTmp = "casa/com/tmp"; // topic per mandare alla dashboard la temperatura rilevata
const char* topicFinestra = "casa/com/finestra"; // topic per ricevere il comando per aprire la finestra
const char* topicFinestraInit = "casa/init/finesrta"; // topic per resettare la finestra a chiusa all'avvio del programma per syncare dashboard e board fisica
const char* topicLuceInit = "casa/init/luce"; // topic per resettare la luce a chiusa all'avvio del programma per syncare dashboard e board fisica
const char* topicECO2 = "casa/com/eco2";
const char* topicTVOC = "casa/com/tvoc";
const char* topicVentola = "casa/com/ventilazione";
const char* topicVentolaInit = "casa/com/ventola";
const char* topicModVentola = "casa/mod/ventola";
const char* topicModFinestra = "casa/mod/finestra";
const char* topicModeLuce = "casa/mod/luce";


void callback(char* topic, uint8_t* payload, unsigned int length) {
  Serial.print("Messaggio arrivato [");  Serial.print(topic);  Serial.print("] ");
  String localTopic = topic;
  String comando = "";
  for (int i = 0; i < length; i++) {
    comando += (char)payload[i];
  }

  Serial.print("Comando = "); Serial.println(comando);

  if(localTopic == "casa/com/finestra") {
    Serial.println("Dentro");
    int value = comando.toInt();
    servo.write(value);
  }

  // spengo la luce
  if(comando == "offLuce") {
    digitalWrite(LIGHT, LOW);
    statoLed = false;
  } else if(comando == "offLuce" && statoLuce == 0) {
    Serial.println("Comando ignorato, luce in automatico.");
  } 

  // accendo la luce
  if(comando == "onLuce") {
    digitalWrite(LIGHT, HIGH);
    statoLed = true;
  } else if(comando == "onLuce" && statoLuce == 0) {
    Serial.println("Comando ignorato, luce in automatico.");
  }

  // apro a finestra
  if(comando == "openWindow") {
    servo.write(0);
    // Serial.println("Finestra aperta (finestra MANUALE)");
  } else if(comando == "openWindow" && statoFinestra == 0) {
    Serial.println("Comando ignorato, finestra in automatico.");
  }

  // chiudo la finestra
  if(comando == "closeWindow") {
    servo.write(90);
    // Serial.println("Finestra chiusa (finestra MANUALE)");
  } else if(comando == "closeWindow" && statoFinestra == 0) {
    Serial.println("Comando ignorato, finestra in automatico.");
  }

  // accendo la ventola SE lo stato della ventola è in manuale
  if(comando == "onVentola") {
    digitalWrite(VENTOLA, HIGH);
    // Serial.println("Ventola accesa (ventola MANUALE)");
  } else if(comando == "onVentola" && statoVentola == 0) {
    Serial.println("Comando ignorato, ventola in automatico.");
  }

  // spengo la ventola SE lo stato della ventola è in manuale
  if(comando == "offVentola") {
    digitalWrite(VENTOLA, LOW);
    // Serial.println("Ventola spenta (ventola MANUALE)");
  } else if(comando == "offVentola" && statoVentola == 0) {
    Serial.println("Comando ignorato, ventola in automatico.");
  }

  // imposto la ventola in modalità manuale (comandata da dashboard)
  if(comando == "manualVentola") {
    statoVentola = 1; // ovvero che la ventola viene comandata in manuale
    Serial.print("Ventola in modalità manuale, statoVentola: "); Serial.println(statoVentola);
  }

  // imposto  la ventola in modalità automatica (comandata da sensore)
  if (comando == "autoVentola") {
    statoVentola = 0; // ovvero la ventola viene comandata dal sensore
    Serial.print("Ventola in modalità automatica, statoVentola: "); Serial.println(statoVentola); 
  }

  // imposto la finestra in modalità manuale
  if(comando == "manualFinestra") {
    statoFinestra = 1;
    Serial.print("Finestra in modalità manuale, statoFinestra: "); Serial.println(statoFinestra);
  }

  // imposto la finestra in modalità automatica
  if(comando == "autoFinestra") {
    statoFinestra = 0;
    Serial.print("Finestra in modalità automatica, statoFinestra: "); Serial.println(statoFinestra);
  }

  // imposto la luce in modalità automatica
  if(comando == "autoLuce") {
    statoLuce = 0;
    Serial.print("Luce in modalità automatica, statoLuce: "); Serial.println(statoLuce);
  }

  // imposto la luce in modalità manuale
  if(comando == "manualLuce") {
    statoLuce = 1;
    Serial.print("Luce in modalità manuale, statoLuce: "); Serial.println(statoLuce);
  }
}

void leggiTemp() {
  char tmpString[30] = "";
  char humString[30] = "";
  float hum = dht.getHumidity();
  float tmp = dht.getTemperature();
  dtostrf(hum, 6, 2, humString);
  dtostrf(tmp, 6, 2, tmpString);
  client.publish(topicHum, humString);
  client.publish(topicTmp, tmpString);
  // if(tmp > 26 || hum > 55 && statoVentola == 0) {
  //   ricircoloAriaOn();
  // } else if(tmp < 26 || hum < 55 && statoVentola == 0) {
  //   ricircoloAriaOff();
  // }
}

void reconnect() { 
  while (!client.connected()) {
    Serial.print("Provando a connettersi ad MQTT...");
    // Attempt to connect
       String clientId = "ESP32Client";
       clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connesso");
      client.subscribe(topicLuceSub);
      client.subscribe(topicVentola);
      client.subscribe(topicFinestra);
      client.subscribe(topicModVentola);
      client.subscribe(topicModFinestra);
      client.subscribe(topicModeLuce);
    } else {
      Serial.print("fallito, rc=");
      Serial.print(client.state());
    }
  }
}

void airQuality() {
  char eco2String[30] = "";
  char tvocString[30] = "";

  if (sgp.IAQmeasure()) {
    float TVOC = sgp.TVOC;
    float eCO2 = sgp.eCO2;
    dtostrf(TVOC, 6, 2, tvocString);
    dtostrf(eCO2, 6, 2, eco2String);
    // Serial.print("TVOC: "); Serial.print(TVOC); Serial.println(" ppb");
    // Serial.print("eCO2: "); Serial.print(eCO2); Serial.println(" ppm");
    client.publish(topicECO2, eco2String);
    client.publish(topicTVOC, tvocString);
  } else {
    Serial.println("Errore durante la lettura del sensore SGP30");
  }
}

void ricircoloAriaOn() {
  digitalWrite(VENTOLA, HIGH);
  servo.write(0);
}

void ricircoloAriaOff() {
    digitalWrite(VENTOLA, LOW);
    servo.write(90);
}

bool findAndConnect(const char* targetSSID, const char* targetPassword) {
  for (int i = 0; i < WiFi.scanNetworks(); i++) {
    if (WiFi.SSID(i) == targetSSID) {
      Serial.println("Trovata rete: " + String(targetSSID));
      WiFi.begin(targetSSID, targetPassword);
      while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connessione in corso...");
      }
      return true;
    }
  }
  return false;
}

/* ----- SETUP ----- */
void setup() {
  Wire.begin(SDA, SCL);
  dht.setup(DHTPIN, DHTTYPE);
  Serial.begin(115200);
  pinMode(LIGHT, OUTPUT);
  pinMode(VENTOLA, OUTPUT);
  servo.attach(FINESTRA);
  sgp.begin();
  if (!sgp.begin()) {
    Serial.println("Errore durante l'inizializzazione del sensore SGP30");
    while (1);
  }
  Serial.println("Inizializzazione SGP30 completata.");

 // Scansione delle reti Wi-Fi circostanti
  Serial.println("Scansione reti Wi-Fi...");
  int numNetworks = WiFi.scanNetworks();

  if (numNetworks == 0) {
    Serial.println("Nessuna rete Wi-Fi rilevata.");
    while (1) {
      delay(1000);
    }
  } else {
    Serial.println(numNetworks + " reti Wi-Fi rilevate.");

    // Cerca e connettiti a una delle due reti disponibili
    if (findAndConnect(ssid1, password1) || findAndConnect(ssid2, password2)) {
      Serial.println("Connessione Wi-Fi riuscita.");
      
      // Connessione al broker MQTT
      client.setServer(mqtt_server, 1883);
      client.setCallback(callback);

      while (!client.connected()) {
        Serial.println("Connessione a MQTT...");
        if (client.connect("ESP32Client")) {
          Serial.println("Connesso a MQTT.");
          client.subscribe(topicLuceSub);
          client.subscribe(topicVentola);
          client.subscribe(topicFinestra);
          client.subscribe(topicModVentola);
          client.subscribe(topicModFinestra);
          client.subscribe(topicModeLuce);
        } else {
          Serial.println("Connessione a MQTT fallita.");
        }
      }
    } else {
      Serial.println("Connessione Wi-Fi fallita per entrambe le reti.");
      while (1) {
        delay(1000);
      }
    }
  }
  reconnect();
  // client.publish(topicFinestraInit, "closeWindow");
  // client.publish(topicLuceInit, "offLuce");
  // digitalWrite(VENTOLA, LOW);
  // digitalWrite(LIGHT, LOW);
  // servo.write(90);
}

/* ----- LOOP ----- */
void loop() {
  leggiTemp(); 
  airQuality();
  // rete wifi
  if(!client.connected()) {
    reconnect();
  }
  client.loop();
}
