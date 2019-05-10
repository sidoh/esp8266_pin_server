#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>

#ifndef _SETTINGS_H_INCLUDED
#define _SETTINGS_H_INCLUDED

#define SETTINGS_FILE  "/config.json"
#define SETTINGS_TERMINATOR '\0'
#define DEFAULT_MQTT_PORT 1883

#define XQUOTE(x) #x
#define QUOTE(x) XQUOTE(x)

#ifndef FIRMWARE_VARIANT
#define FIRMWARE_VARIANT unknown
#endif

#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION unknown
#endif

class Settings {
public:
  Settings() :
    adminUsername(""),
    adminPassword("")
  { }

  ~Settings() { }

  bool isAuthenticationEnabled() const;
  const String& getUsername() const;
  const String& getPassword() const;

  static void load(Settings& settings);

  void save();
  void serialize(Stream& stream, const bool prettyPrint = false);
  void patch(JsonObject obj);

  String mqttServer();
  uint16_t mqttPort();

  String adminUsername;
  String adminPassword;
  String _mqttServer;
  String mqttUsername;
  String mqttPassword;
  String mqttCommandTopicPattern;
  String mqttStateTopicPattern;
  std::vector<uint8_t> updatePins;
  std::vector<uint8_t> outputPins;
  std::vector<uint8_t> dallasTempPins;
  String mqttTempTopicPattern;
  time_t thermometerUpdateInterval;

protected:
  size_t _autoRestartPeriod;

  template <typename T>
  void setIfPresent(JsonObject obj, const char* key, T& var) {
    if (obj.containsKey(key)) {
      JsonVariant val = obj[key];
      var = val.as<T>();
    }
  }

  template<typename T>
  static void copyFrom(JsonArray arr, std::vector<T> vec) {
    for (typename std::vector<T>::const_iterator it = vec.begin(); it != vec.end(); ++it) {
      arr.add(*it);
    }
  }

  template<typename T>
  static void copyTo(JsonArray arr, std::vector<T> vec) {
    for (size_t i = 0; i < arr.size(); ++i) {
      JsonVariant val = arr[i];
      vec.push_back(val.as<T>());
    }
  }
};

#endif
