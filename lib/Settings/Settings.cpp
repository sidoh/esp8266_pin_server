#include <Settings.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <algorithm>

#define PORT_POSITION(s) ( s.indexOf(':') )

void Settings::patch(JsonObject parsedSettings) {
  if (!parsedSettings.isNull()) {
    this->setIfPresent(parsedSettings, "admin_username", adminUsername);
    this->setIfPresent(parsedSettings, "admin_password", adminPassword);
    this->setIfPresent(parsedSettings, "mqtt_server", _mqttServer);
    this->setIfPresent(parsedSettings, "mqtt_username", mqttUsername);
    this->setIfPresent(parsedSettings, "mqtt_password", mqttPassword);
    this->setIfPresent(parsedSettings, "mqtt_command_topic_pattern", mqttCommandTopicPattern);
    this->setIfPresent(parsedSettings, "mqtt_state_topic_pattern", mqttStateTopicPattern);
    this->setIfPresent(parsedSettings, "mqtt_temp_topic_pattern", mqttTempTopicPattern);
    this->setIfPresent(parsedSettings, "thermometer_update_interval", thermometerUpdateInterval);

    this->updatePins.clear();
    if (parsedSettings.containsKey("update_pins")) {
      JsonArray updatePins = parsedSettings["update_pins"];
      copyTo(updatePins, this->updatePins);
    }

    this->outputPins.clear();
    if (parsedSettings.containsKey("output_pins")) {
      JsonArray outputPins = parsedSettings["output_pins"];
      copyTo(outputPins, this->outputPins);
    }

    this->dallasTempPins.clear();
    if (parsedSettings.containsKey("dallas_temp_pins")) {
      JsonArray dallasTempPins = parsedSettings["dallas_temp_pins"];
      copyTo(dallasTempPins, this->dallasTempPins);
    }
  }
}

void Settings::load(Settings& settings) {
  if (SPIFFS.exists(SETTINGS_FILE)) {
    DynamicJsonDocument jsonBuffer(2048);

    File f = SPIFFS.open(SETTINGS_FILE, "r");
    deserializeJson(jsonBuffer, f);
    f.close();

    settings.patch(jsonBuffer.as<JsonObject>());
  } else {
    settings.save();
  }
}

void Settings::save() {
  File f = SPIFFS.open(SETTINGS_FILE, "w");

  if (!f) {
    Serial.println(F("Opening settings file failed"));
  } else {
    serialize(f);
    f.close();
  }
}

void Settings::serialize(Stream& stream, const bool prettyPrint) {
  DynamicJsonDocument jsonBuffer(2048);
  JsonObject root = jsonBuffer.as<JsonObject>();

  root["admin_username"] = this->adminUsername;
  root["admin_password"] = this->adminPassword;
  root["mqtt_server"] = this->_mqttServer;
  root["mqtt_username"] = this->mqttUsername;
  root["mqtt_password"] = this->mqttPassword;
  root["mqtt_command_topic_pattern"] = this->mqttCommandTopicPattern;
  root["mqtt_state_topic_pattern"] = this->mqttStateTopicPattern;
  root["mqtt_temp_topic_pattern"] = this->mqttTempTopicPattern;
  root["thermometer_update_interval"] = this->thermometerUpdateInterval;

  JsonArray updatePins = root.createNestedArray("update_pins");
  copyFrom(updatePins, this->updatePins);

  JsonArray outputPins = root.createNestedArray("output_pins");
  copyFrom(outputPins, this->outputPins);

  JsonArray dallasTempPins = root.createNestedArray("dallas_temp_pins");
  copyFrom(dallasTempPins, this->dallasTempPins);

  if (prettyPrint) {
    serializeJsonPretty(root, stream);
  } else {
    serializeJson(root, stream);
  }
}

String Settings::mqttServer() {
  int pos = PORT_POSITION(_mqttServer);

  if (pos == -1) {
    return _mqttServer;
  } else {
    return _mqttServer.substring(0, pos);
  }
}

uint16_t Settings::mqttPort() {
  int pos = PORT_POSITION(_mqttServer);

  if (pos == -1) {
    return DEFAULT_MQTT_PORT;
  } else {
    return atoi(_mqttServer.c_str() + pos + 1);
  }
}

bool Settings::isAuthenticationEnabled() const {
  return adminUsername.length() > 0 && adminPassword.length() > 0;
}

const String& Settings::getUsername() const {
  return adminUsername;
}

const String& Settings::getPassword() const {
  return adminPassword;
}