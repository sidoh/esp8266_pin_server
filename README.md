# esp8266_pin_server
Small REST/MQTT gateway to read/write GPIO pins on an ESP8266

## Configuring

Use the `/settings` route to configure pin modes, and MQTT parameters:

#### Pin Modes

Pins will be in input mode by default.  To change that:


```
curl -X PUT -H 'Content-Type: application/json' \
  -d '{"output_pins": [4, 5]}' \
  http://your-pin-server/settings
```

#### MQTT

```
curl -X PUT -H 'Content-Type: application/json' \
  -d '{
    "mqtt_server":"mqtt-server:1883",
    "mqtt_command_topic_pattern":"pin-servers/pin-server1/commands/:pin",
    "mqtt_state_topic_pattern":"pin-servers/pin-server1/states/:pin",
    "mqtt_username":"mqtt-user",
    "mqtt_password":"hunter2"
  }' \
  http://your-pin-server/settings
```

#### Getting MQTT state updates

In addition to configuring MQTT, if you want state updates when a pin changes value, add the pin to `update_pins`, which will attach an interrupt handler for the appropriate pin:

```
curl -X PUT -H 'Content-Type: application/json' \
  -d '{"update_pins": [4, 5]}' \
  http://your-pin-server/settings
```

## REST Routes

* `GET /pins/:pin_number`. Returns 0 (`LOW`) or 1 (`HIGH`).
* `PUT /pins/:pin_number`. Changes the state of `pin_number`. The body should be JSON of the form:
  ```
  {"action":"...",...}
  ```
* `GET /settings`.
* `PUT /settings`. Patches provided settings.  Accepts JSON object.

#### Examples

1. `{"action":"set","value":0}`
1. `{"action":"flap"}`
1. `{"action":"flap","until":{"pin":11,"value":0}}`
1. `{"action":"toggle"}`

```
$ curl -X PUT -d '{"action":"set","value":0}' -H'Content-Type: application/json' server/pins/11
$ curl -X PUT -d '{"action":"flap"}' -H'Content-Type: application/json' server/pins/13
$ curl -X PUT -d '{"action":"flap","until":{"pin":11,"value":0}}' -H'Content-Type: application/json' server/pins/13
$ curl -X PUT -d '{"action":"toggle"}' -H'Content-Type: application/json' server/pins/11
```

## MQTT Control

You can also control pins via MQTT if the setting key `mqtt_command_topic_pattern` is configured.  The message should be JSON in the same form that the REST API accepts. 

If this is set to `pin-servers/pin-server1/states/:pin`, for example, you could set pin 11 HIGH with this:

```
$ mosquitto_pub -h mqtt-server -p 1883 -u mqtt-user -P hunter2 -t 'pin-severs/pin-server1/commands/13' '{"action":"set","value":1}'
```
