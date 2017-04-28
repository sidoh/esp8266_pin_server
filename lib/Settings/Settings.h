#include <Arduino.h>
#include <StringStream.h>
#include <ArduinoJson.h>

#ifndef _SETTINGS_H_INCLUDED
#define _SETTINGS_H_INCLUDED

#define SETTINGS_FILE  "/config.json"
#define SETTINGS_TERMINATOR '\0'
#define DEFAULT_MQTT_PORT 1883

class Settings {
public:
  Settings() :
    adminUsername(""),
    adminPassword("")
  { }

  ~Settings() {
    if (updatePins) {
      delete updatePins;
    }
  }

  bool hasAuthSettings();

  static void deserialize(Settings& settings, String json);
  static void load(Settings& settings);

  void save();
  void serialize(Stream& stream, const bool prettyPrint = false);
  void patch(JsonObject& obj);

  String mqttServer();
  uint16_t mqttPort();

  String adminUsername;
  String adminPassword;
  String _mqttServer;
  String mqttUsername;
  String mqttPassword;
  String mqttCommandTopicPattern;
  String mqttStateTopicPattern;
  uint8_t* updatePins;
  size_t numUpdatePins;
  uint8_t* outputPins;
  size_t numOutputPins;

protected:
  size_t _autoRestartPeriod;

  template <typename T>
  void setIfPresent(JsonObject& obj, const char* key, T& var) {
    if (obj.containsKey(key)) {
      var = obj.get<T>(key);
    }
  }
};

#endif
