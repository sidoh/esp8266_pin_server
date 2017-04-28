#include <inttypes.h>
#include <ArduinoJson.h>

#ifndef PIN_HANDLER_H
#define PIN_HANDLER_H

#ifndef MAX_FLAP_TRIES
#define MAX_FLAP_TRIES 10
#endif

#ifndef FLAP_TRIES_SLEEP_PERIOD
#define FLAP_TRIES_SLEEP_PERIOD 100
#endif

class PinHandler {
public:
  void flap(uint8_t pin);
  void flapUntil(uint8_t pin, uint8_t readPin, uint8_t expectedValue);
  void toggle(uint8_t pin);
  void handle(uint8_t pin, JsonObject& request);
};

#endif
