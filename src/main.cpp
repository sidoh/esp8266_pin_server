#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <PinHandler.h>
#include <WebServer.h>
#include <Settings.h>
#include <MqttClient.h>
#include <FS.h>
#include <LinkedList.h>
#include <ArduinoJson.h>

#include <OneWire.h>
#include <DallasTemperature.h>
#include <TempIface.h>

#include <vector>
#include <map>

Settings settings;
PinHandler pinHandler;
WebServer server(settings, pinHandler);

WiFiManager wifiManager;
WiFiClient tcpClient;
MqttClient* mqttClient;
volatile int8_t interruptPin;
std::vector<TempIface*> thermometers;
std::map<uint8_t, uint8_t> lastPinValues;
unsigned long lastTempUpdate = 0;

typedef void(*InterruptHandler)();

void HandleInterrupt(uint8_t pin) {
  interruptPin = pin;
}

inline void Handler1() { HandleInterrupt(1); }
void Handler2() { HandleInterrupt(2); }
void Handler3() { HandleInterrupt(3); }
void Handler4() { HandleInterrupt(4); }
void Handler5() { HandleInterrupt(5); }
void Handler6() { HandleInterrupt(6); }
void Handler7() { HandleInterrupt(7); }
void Handler8() { HandleInterrupt(8); }
void Handler9() { HandleInterrupt(9); }
void Handler10() { HandleInterrupt(10); }
void Handler11() { HandleInterrupt(11); }
void Handler12() { HandleInterrupt(12); }
void Handler13() { HandleInterrupt(13); }
void Handler14() { HandleInterrupt(14); }
void Handler15() { HandleInterrupt(15); }
void Handler16() { HandleInterrupt(16); }

InterruptHandler getHandler(uint8_t pin) {
  switch (pin) {
    case 1: return Handler1;
    case 2: return Handler2;
    case 3: return Handler3;
    case 4: return Handler4;
    case 5: return Handler5;
    case 6: return Handler6;
    case 7: return Handler7;
    case 8: return Handler8;
    case 9: return Handler9;
    case 10: return Handler10;
    case 11: return Handler11;
    case 12: return Handler12;
    case 13: return Handler13;
    case 14: return Handler14;
    case 15: return Handler15;
    case 16: return Handler16;
  }
}

void publishTemperature(String id, float temp) {
  const String& topicPattern = settings.mqttTempTopicPattern;

  if (mqttClient && topicPattern.length() > 0) {
    String topic = topicPattern;
    topic.replace(":id", id);
    mqttClient->publish(topic.c_str(), String(temp).c_str());
  }
}

void publishMqttUpdate(uint8_t pin) {
  if (mqttClient) {
    String topicPattern = String(settings.mqttStateTopicPattern);
    topicPattern.replace(":pin", String(pin));
    mqttClient->publish(topicPattern.c_str(), String(digitalRead(pin)).c_str());
  }
}

void setup() {
  Serial.begin(9600);
  SPIFFS.begin();
  Settings::load(settings);

  wifiManager.autoConnect();
  pinHandler.onPinChange(publishMqttUpdate);

  if (settings.mqttServer().length() > 0) {
    mqttClient = new MqttClient(settings, pinHandler);
    mqttClient->begin();
  }

  for (size_t i = 0; i < settings.updatePins.size(); i++) {
    uint8_t pin = settings.updatePins[i];
    pinMode(pin, INPUT);

    attachInterrupt(
      digitalPinToInterrupt(pin),
      getHandler(pin),
      CHANGE
    );

    publishMqttUpdate(pin);
  }

  for (size_t i = 0; i < settings.outputPins.size(); i++) {
    uint8_t pin = settings.outputPins[i];
    pinMode(pin, OUTPUT);
    digitalWrite(pin, 0);
  }

  for (size_t i = 0; i < settings.dallasTempPins.size(); i++) {
    OneWire* oneWireBus = new OneWire(settings.dallasTempPins[i]);
    DallasTemperature* thermometer = new DallasTemperature(oneWireBus);
    TempIface* tempIface = new TempIface(thermometer);

    thermometer->begin();
    tempIface->begin();

    thermometers.push_back(tempIface);
  }

  interruptPin = -1;
  server.begin();
}

void loop() {
  server.handleClient();

  if (mqttClient) {
    mqttClient->handleClient();
  }

  noInterrupts();
  if (interruptPin != -1) {
    uint8_t value = digitalRead(interruptPin);

    if (lastPinValues.find(interruptPin) == lastPinValues.end() || lastPinValues[interruptPin] != value) {
      publishMqttUpdate(interruptPin);
      lastPinValues[interruptPin] = value;
    }

    interruptPin = -1;
  }
  interrupts();

  if (lastTempUpdate == 0 || (lastTempUpdate + settings.thermometerUpdateInterval) < millis()) {
    for (std::vector<TempIface*>::iterator it = thermometers.begin(); it != thermometers.end(); ++it) {
      (*it)->refreshTemps();
      const std::map<String, float>& temps = (*it)->getCurrentTemps();

      for (std::map<String, float>::const_iterator tempIt = temps.begin(); tempIt != temps.end(); ++tempIt) {
        publishTemperature(tempIt->first, tempIt->second);
      }
    }

    lastTempUpdate = millis();
  }
}
