# esp8266_pin_server
Small REST gateway to read/write GPIO pins on an ESP8266

## Routes

* `GET /pins/:pin_number`. Returns 0 (`LOW`) or 1 (`HIGH`).
* `PUT /pins/:pin_number`. Changes the state of `pin_number`. The body should be one of `HIGH`, `LOW`, `TOGGLE`, or `FLAP`. `FLAP` toggles the state for 100ms and then toggles it back.

## Example

```
$ curl -vvv -X PUT --data-binary 'FLAP' server/pins/13
* Hostname was NOT found in DNS cache
*   Trying 192.168.1.100...
* Connected to 192.168.1.100 (192.168.1.100) port 80 (#0)
> PUT /pins/13 HTTP/1.1
> User-Agent: curl/7.35.0
> Host: 192.168.1.100
> Accept: */*
> Content-Length: 4
> Content-Type: application/x-www-form-urlencoded
>
* upload completely sent off: 4 out of 4 bytes
< HTTP/1.1 200 OK
< Content-Type: text/plain
< Content-Length: 1
< Connection: close
< Access-Control-Allow-Origin: *
<
* Closing connection 0
0%
```
