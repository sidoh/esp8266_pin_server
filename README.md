# esp8266_pin_server
Small REST gateway to read/write GPIO pins on an ESP8266

## Routes

* `GET /pins/:pin_number`. Returns 0 (`LOW`) or 1 (`HIGH`).
* `PUT /pins/:pin_number`. Changes the state of `pin_number`. The body should be JSON of the form:
  ```
  {"action":"...",...}
  ```
* `GET /settings`.
* `PUT /settings`. Patches provided settings.  Accepts JSON object.

#### Examples

1. `{"action":"set","value":0}`
1. `{"action":"flap"}`, `{"action":"flap","until":{"pin":11,"value":0}}`
1. `{"action":"toggle"}`

## Example

```
$ curl -X PUT -d '{"action":"flap"}' -H'Content-Type: application/json' server/pins/13
```
