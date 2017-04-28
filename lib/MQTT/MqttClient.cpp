#include <MqttClient.h>
#include <TokenIterator.h>
#include <UrlTokenBindings.h>
#include <IntParsing.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>

MqttClient::MqttClient(Settings& settings, PinHandler& pinHandler)
  : settings(settings),
    pinHandler(pinHandler),
    lastConnectAttempt(0)
{
  String strDomain = settings.mqttServer();
  this->domain = new char[strDomain.length() + 1];
  strcpy(this->domain, strDomain.c_str());

  this->mqttClient = new PubSubClient(tcpClient);
}

MqttClient::~MqttClient() {
  mqttClient->disconnect();
  delete this->domain;
}

void MqttClient::begin() {
#ifdef MQTT_DEBUG
  printf(
    "MqttClient - Connecting to: %s\nparsed:%s:%u\n",
    settings._mqttServer.c_str(),
    settings.mqttServer().c_str(),
    settings.mqttPort()
  );
#endif

  mqttClient->setServer(this->domain, settings.mqttPort());
  mqttClient->setCallback(
    [this](char* topic, byte* payload, int length) {
      this->publishCallback(topic, payload, length);
    }
  );
  reconnect();
}

void MqttClient::publish(const char* topic, const char* message) {
  mqttClient->publish(topic, message);
}

bool MqttClient::connect() {
  char nameBuffer[30];
  sprintf_P(nameBuffer, PSTR("milight-hub-%u"), ESP.getChipId());

#ifdef MQTT_DEBUG
    Serial.println(F("MqttClient - connecting"));
#endif

  if (settings.mqttUsername.length() > 0) {
    return mqttClient->connect(
      nameBuffer,
      settings.mqttUsername.c_str(),
      settings.mqttPassword.c_str()
    );
  } else {
    return mqttClient->connect(nameBuffer);
  }
}

void MqttClient::reconnect() {
  if (lastConnectAttempt > 0 && (millis() - lastConnectAttempt) < MQTT_CONNECTION_ATTEMPT_FREQUENCY) {
    return;
  }

  if (! mqttClient->connected()) {
    if (connect()) {
      subscribe();

#ifdef MQTT_DEBUG
      Serial.println(F("MqttClient - Successfully connected to MQTT server"));
#endif
    } else {
      Serial.println(F("ERROR: Failed to connect to MQTT server"));
    }
  }

  lastConnectAttempt = millis();
}

void MqttClient::handleClient() {
  reconnect();
  mqttClient->loop();
}

void MqttClient::subscribe() {
  String topic = settings.mqttCommandTopicPattern;

  topic.replace(":pin", "+");

#ifdef MQTT_DEBUG
  printf("MqttClient - subscribing to topic: %s\n", topic.c_str());
#endif

  mqttClient->subscribe(topic.c_str());
}

void MqttClient::publishCallback(char* topic, byte* payload, int length) {
  char cstrPayload[length + 1];
  cstrPayload[length] = 0;
  memcpy(cstrPayload, payload, sizeof(byte)*length);

#ifdef MQTT_DEBUG
  printf("MqttClient - Got message on topic: %s\n%s\n", topic, cstrPayload);
#endif

  char topicPattern[settings.mqttCommandTopicPattern.length()+1];
  strcpy(topicPattern, settings.mqttCommandTopicPattern.c_str());

  TokenIterator patternIterator(topicPattern, settings.mqttCommandTopicPattern.length(), '/');
  TokenIterator topicIterator(topic, strlen(topic), '/');
  UrlTokenBindings tokenBindings(patternIterator, topicIterator);

  StaticJsonBuffer<400> buffer;
  JsonObject& request = buffer.parse(cstrPayload);

  uint8_t pin = atoi(tokenBindings.get("pin"));
  pinHandler.handle(pin, request);
}
