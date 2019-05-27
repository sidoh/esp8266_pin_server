#include <Arduino.h>
#include <PinHandler.h>
#include <ArduinoJson.h>

PinHandler::PinHandler()
  : callbackFn(NULL)
{ }

void PinHandler::flap(uint8_t pin) {
  uint8_t value = digitalRead(pin);

  digitalWrite(pin, value == 0 ? 1 : 0);
  delay(100);
  digitalWrite(pin, value);
}

void PinHandler::flapUntil(uint8_t pin, uint8_t readPin, uint8_t expectedValue) {
  uint8_t tries = 0;

  while (tries++ < MAX_FLAP_TRIES && digitalRead(readPin) != expectedValue) {
    flap(pin);

    if (digitalRead(readPin) != expectedValue) {
      delay(FLAP_TRIES_SLEEP_PERIOD);
    }
  }
}

void PinHandler::toggle(uint8_t pin) {
  uint8_t value = digitalRead(pin);
  digitalWrite(pin, value == 0 ? 1 : 0);
}

void PinHandler::handle(uint8_t pin, JsonObject request) {
  if (request.isNull() || ! request.containsKey("action")) {
    Serial.print("Invalid input: ");
    serializeJson(request, Serial);
    return;
  }

  const String action = request["action"];

  if (action.equalsIgnoreCase("set")) {
    uint8_t value = request["value"];
    digitalWrite(pin, value);
  } else if (action.equalsIgnoreCase("flap")) {
    if (request.containsKey("until")) {
      const JsonObject& untilParams = request["until"];
      uint8_t readPin = untilParams["pin"];
      uint8_t value = untilParams["value"];

      flapUntil(pin, readPin, value);
    } else {
      flap(pin);
    }
  } else if (action.equalsIgnoreCase("toggle")) {
    toggle(pin);
  } else if (request.containsKey("mode")) {
    String mode = request["mode"];
    pinMode(pin, mode.equalsIgnoreCase("input") ? INPUT : OUTPUT);
  }
}

void PinHandler::onPinChange(TPinHandlerCallback callbackFn) {
  this->callbackFn = callbackFn;
}
