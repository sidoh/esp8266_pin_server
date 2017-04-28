#include <Settings.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <PinHandler.h>

#ifndef MQTT_CONNECTION_ATTEMPT_FREQUENCY
#define MQTT_CONNECTION_ATTEMPT_FREQUENCY 5000
#endif

#ifndef _MQTT_CLIENT_H
#define _MQTT_CLIENT_H

class MqttClient {
public:
  MqttClient(Settings& settings, PinHandler& pinHandler);
  ~MqttClient();

  void begin();
  void handleClient();
  void reconnect();
  void publish(const char* topic, const char* message);

private:
  WiFiClient tcpClient;
  PubSubClient* mqttClient;
  Settings& settings;
  PinHandler& pinHandler;
  char* domain;
  unsigned long lastConnectAttempt;

  bool connect();
  void subscribe();
  void publishCallback(char* topic, byte* payload, int length);
};

#endif
