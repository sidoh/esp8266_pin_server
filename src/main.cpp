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

WebServer server(80);
WiFiManager wifiManager;
Settings settings;
WiFiClient tcpClient;
MqttClient* mqttClient;
PinHandler pinHandler;
volatile int8_t interruptPin;
std::vector<TempIface*> thermometers;
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

  server.on("/about", HTTP_GET, []() {
    // Measure before allocating buffers
    uint32_t freeHeap = ESP.getFreeHeap();

    StaticJsonBuffer<150> buffer;
    JsonObject& res = buffer.createObject();

    res["version"] = QUOTE(FIRMWARE_VERSION);
    res["variant"] = QUOTE(FIRMWARE_VARIANT);
    res["signal_strength"] = WiFi.RSSI();
    res["free_heap"] = freeHeap;
    res["sdk_version"] = ESP.getSdkVersion();

    String body;
    res.printTo(body);

    server.send(200, "application/json", body);
  });

  server.onPattern("/pins/:pin", HTTP_PUT, [](UrlTokenBindings* bindings){
    String body = server.arg("plain");
    StaticJsonBuffer<400> buffer;

    JsonObject& request = buffer.parse(body);
    uint8_t pin = atoi(bindings->get("pin"));
    pinMode(pin, OUTPUT);

    pinHandler.handle(pin, request);

    server.send(200, "text/plain", String(digitalRead(pin)));
  });

  server.onPattern("/pins/:pin", HTTP_GET, [](UrlTokenBindings* bindings){
    uint8_t pin = atoi(bindings->get("pin"));
    server.send(200, "text/plain", String(digitalRead(pin)));
  });

  server.on("/settings", HTTP_PUT,
    []() {
      StaticJsonBuffer<400> buffer;
      JsonObject& obj = buffer.parse(server.arg("plain"));
      settings.patch(obj);
      settings.save();

      String body;
      StringStream stream(body);
      settings.serialize(stream, true);

      server.send(200, "application/json", body);

      delay(100);

      ESP.restart();
    }
  );

  server.on("/settings", HTTP_GET,
    []() {
      String body;
      StringStream stream(body);
      settings.serialize(stream, true);

      server.send(200, "application/json", body);
    }
  );

  server.on("/firmware", HTTP_POST,
    [](){
      server.sendHeader("Connection", "close");
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(200, "text/plain", (Update.hasError())?"FAIL":"OK");

      delay(100);

      ESP.restart();
    },
    [](){
      HTTPUpload& upload = server.upload();
      if(upload.status == UPLOAD_FILE_START){
        WiFiUDP::stopAll();
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if(!Update.begin(maxSketchSpace)){//start with max available size
          Update.printError(Serial);
        }
      } else if(upload.status == UPLOAD_FILE_WRITE){
        if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
          Update.printError(Serial);
        }
      } else if(upload.status == UPLOAD_FILE_END){
        if(Update.end(true)){ //true to set the size to the current progress
        } else {
          Update.printError(Serial);
        }
      }
      yield();
    }
  );

  for (size_t i = 0; i < settings.numUpdatePins; i++) {
    uint8_t pin = settings.updatePins[i];
    pinMode(pin, INPUT);

    attachInterrupt(
      digitalPinToInterrupt(pin),
      getHandler(pin),
      CHANGE
    );

    publishMqttUpdate(pin);
  }

  for (size_t i = 0; i < settings.numOutputPins; i++) {
    uint8_t pin = settings.outputPins[i];
    pinMode(pin, OUTPUT);
    digitalWrite(pin, 0);
  }

  for (size_t i = 0; i < settings.numDallasTempPins; i++) {
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
    publishMqttUpdate(interruptPin);
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
