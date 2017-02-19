#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <WebServer.h>

WebServer server(80);
WiFiManager wifiManager;

void setup() {
  Serial.begin(115200);
  
  wifiManager.autoConnect();
  
  server.onPattern("/pins/:pin", HTTP_PUT, [](UrlTokenBindings* bindings){
    String request = server.arg("plain");
    uint8_t pin = bindings->get("pin").toInt();
    uint8_t state = digitalRead(pin);
    
    pinMode(pin, OUTPUT);
    
    if (request.equalsIgnoreCase("high")) {
      state = HIGH;
    } else if (request.equalsIgnoreCase("low")) {
      state = LOW;
    } else if (request.equalsIgnoreCase("toggle")) {
      state = state == HIGH ? LOW : HIGH;
    } else if (request.equalsIgnoreCase("flap")) {
      digitalWrite(pin, state == HIGH ? LOW : HIGH);
      delay(100);
    } else {
      server.send(400, "text/plain", "Invalid argument. Must be one of: HIGH, LOW, TOGGLE, FLAP");
      return;
    }
    
    digitalWrite(pin, state);
    server.send(200, "text/plain", String(state));
  });
  
  server.onPattern("/pins/:pin", HTTP_GET, [](UrlTokenBindings* bindings){
    uint8_t pin = bindings->get("pin").toInt();
    pinMode(pin, INPUT);
    server.send(200, "text/plain", String(digitalRead(pin)));
  });
  
  server.on("/firmware", HTTP_POST, 
    [](){
      server.sendHeader("Connection", "close");
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(200, "text/plain", (Update.hasError())?"FAIL":"OK");
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
  
  server.begin();
}

void loop() {
  server.handleClient();
}