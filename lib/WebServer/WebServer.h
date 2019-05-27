#ifndef _WEBSERVER_H
#define _WEBSERVER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <RichHttpServer.h>
#include <Settings.h>

#include <PinHandler.h>

using RichHttpConfig = RichHttp::Generics::Configs::EspressifBuiltin;
using RequestContext = RichHttpConfig::RequestContextType;

class WebServer {
public:
  WebServer(Settings& settings, PinHandler& pinHandler);

  void begin();
  void handleClient();

private:
  Settings& settings;
  PassthroughAuthProvider<Settings> authProvider;
  RichHttpServer<RichHttpConfig> server;
  PinHandler& pinHandler;

  void handleAbout(RequestContext& request);

  void handleGetSettings(RequestContext& request);
  void handlePutSettings(RequestContext& request);

  void handleGetPin(RequestContext& request);
  void handlePutPin(RequestContext& request);
};

#endif
