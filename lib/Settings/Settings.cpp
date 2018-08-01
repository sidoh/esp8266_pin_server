#include <Settings.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <algorithm>

#define PORT_POSITION(s) ( s.indexOf(':') )

bool Settings::hasAuthSettings() {
  return adminUsername.length() > 0 && adminPassword.length() > 0;
}

void Settings::deserialize(Settings& settings, String json) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& parsedSettings = jsonBuffer.parseObject(json);
  settings.patch(parsedSettings);
}

void Settings::patch(JsonObject& parsedSettings) {
  if (parsedSettings.success()) {
    this->setIfPresent<String>(parsedSettings, "admin_username", adminUsername);
    this->setIfPresent(parsedSettings, "admin_password", adminPassword);
    this->setIfPresent(parsedSettings, "mqtt_server", _mqttServer);
    this->setIfPresent(parsedSettings, "mqtt_username", mqttUsername);
    this->setIfPresent(parsedSettings, "mqtt_password", mqttPassword);
    this->setIfPresent(parsedSettings, "mqtt_command_topic_pattern", mqttCommandTopicPattern);
    this->setIfPresent(parsedSettings, "mqtt_state_topic_pattern", mqttStateTopicPattern);
    this->setIfPresent(parsedSettings, "mqtt_temp_topic_pattern", mqttTempTopicPattern);
    this->setIfPresent(parsedSettings, "thermometer_update_interval", thermometerUpdateInterval);

    if (parsedSettings.containsKey("update_pins")) {
      if (this->updatePins) {
        delete this->updatePins;
      }

      JsonArray& updatePins = parsedSettings["update_pins"];
      this->numUpdatePins = updatePins.size();
      this->updatePins = new uint8_t[this->numUpdatePins];
      updatePins.copyTo(this->updatePins, this->numUpdatePins);
    }

    if (parsedSettings.containsKey("output_pins")) {
      if (this->outputPins) {
        delete this->outputPins;
      }

      JsonArray& outputPins = parsedSettings["output_pins"];
      this->numOutputPins = outputPins.size();
      this->outputPins = new uint8_t[this->numOutputPins];
      outputPins.copyTo(this->outputPins, this->numOutputPins);
    }

    if (parsedSettings.containsKey("dallas_temp_pins")) {
      if (this->dallasTempPins) {
        delete this->dallasTempPins;
      }

      JsonArray& dallasTempPins = parsedSettings["dallas_temp_pins"];
      this->numDallasTempPins = dallasTempPins.size();
      this->dallasTempPins = new uint8_t[this->numDallasTempPins];
      dallasTempPins.copyTo(this->dallasTempPins, this->numDallasTempPins);
    }
  }
}

void Settings::load(Settings& settings) {
  if (SPIFFS.exists(SETTINGS_FILE)) {
    File f = SPIFFS.open(SETTINGS_FILE, "r");
    String settingsContents = f.readStringUntil(SETTINGS_TERMINATOR);
    f.close();

    deserialize(settings, settingsContents);
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
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root["admin_username"] = this->adminUsername;
  root["admin_password"] = this->adminPassword;
  root["mqtt_server"] = this->_mqttServer;
  root["mqtt_username"] = this->mqttUsername;
  root["mqtt_password"] = this->mqttPassword;
  root["mqtt_command_topic_pattern"] = this->mqttCommandTopicPattern;
  root["mqtt_state_topic_pattern"] = this->mqttStateTopicPattern;
  root["mqtt_temp_topic_pattern"] = this->mqttTempTopicPattern;
  root["thermometer_update_interval"] = this->thermometerUpdateInterval;

  JsonArray& updatePins = jsonBuffer.createArray();
  updatePins.copyFrom(this->updatePins, this->numUpdatePins);
  root["update_pins"] = updatePins;

  JsonArray& outputPins = jsonBuffer.createArray();
  outputPins.copyFrom(this->outputPins, this->numOutputPins);
  root["output_pins"] = outputPins;

  JsonArray& dallasTempPins = jsonBuffer.createArray();
  dallasTempPins.copyFrom(this->dallasTempPins, this->numDallasTempPins);
  root["dallas_temp_pins"] = dallasTempPins;

  if (prettyPrint) {
    root.prettyPrintTo(stream);
  } else {
    root.printTo(stream);
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
