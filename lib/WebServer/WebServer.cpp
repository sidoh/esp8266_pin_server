#include <PinHandler.h>
#include <Settings.h>
#include <WebServer.h>
#include <FS.h>

using namespace std::placeholders;

WebServer::WebServer(Settings& settings, PinHandler& pinHandler)
  : settings(settings)
  , authProvider(settings)
  , server(80, authProvider)
  , pinHandler(pinHandler)
{ }

void WebServer::begin() {
  server
    .buildHandler("/pin/:pin")
    .on(HTTP_GET, std::bind(&WebServer::handleGetPin, this, _1))
    .on(HTTP_PUT, std::bind(&WebServer::handlePutPin, this, _1));

  server
    .buildHandler("/settings")
    .on(HTTP_GET, std::bind(&WebServer::handleGetSettings, this, _1))
    .on(HTTP_PUT, std::bind(&WebServer::handlePutSettings, this, _1));

  server
    .buildHandler("/about")
    .on(HTTP_GET, std::bind(&WebServer::handleAbout, this, _1));

  server
    .buildHandler("/firmware")
    .handleOTA();
}

void WebServer::handleClient() {
  server.handleClient();
}

void WebServer::handleAbout(RequestContext& request) {
  // Measure before allocating buffers
  uint32_t freeHeap = ESP.getFreeHeap();
  JsonObject res = request.response.json.to<JsonObject>();

  res["version"] = QUOTE(FIRMWARE_VERSION);
  res["variant"] = QUOTE(FIRMWARE_VARIANT);
  res["signal_strength"] = WiFi.RSSI();
  res["free_heap"] = freeHeap;
  res["sdk_version"] = ESP.getSdkVersion();
}

void WebServer::handleGetPin(RequestContext& request) {
  uint8_t pinId = atoi(request.pathVariables.get("pin"));

  JsonObject pin = request.response.json.createNestedObject("pin");
  pin["value"] = digitalRead(pinId);
}

void WebServer::handlePutPin(RequestContext& request) {
  JsonObject body = request.getJsonBody().as<JsonObject>();

  uint8_t pin = atoi(request.pathVariables.get("pin"));
  pinMode(pin, OUTPUT);

  pinHandler.handle(pin, body);

  handleGetPin(request);
}

void WebServer::handleGetSettings(RequestContext& request) {
  const char* file = SETTINGS_FILE;

  if (SPIFFS.exists(file)) {
    File f = SPIFFS.open(file, "r");
    server.streamFile(f, "application/json");
    f.close();
  } else {
    request.response.json["error"] = F("Settings file not stored on SPIFFS.  This is an unexpected error!");
    request.response.setCode(500);
  }
}

void WebServer::handlePutSettings(RequestContext& request) {
  JsonObject newSettings = request.getJsonBody().as<JsonObject>();
  settings.patch(newSettings);
  settings.save();

  // just re-serve settings file
  handleGetSettings(request);

  ESP.restart();
}