#include "ESP8266Interface.h"

ESP8266Interface::ESP8266Interface(PinName tx, PinName rx, PinName reset, 
                                   int baud, int timeout) :
        ESP8266(tx, rx, reset, baud, timeout) {
}

bool ESP8266Interface::init() {
    return ESP8266::init();
}

bool ESP8266Interface::connect(const char * ssid, const char * phrase) {
    return ESP8266::connect(ssid, phrase);
}

int ESP8266Interface::disconnect() {
    return ESP8266::disconnect();
}

const char *ESP8266Interface::getIPAddress() {
    return ESP8266::getIPAddress();
}