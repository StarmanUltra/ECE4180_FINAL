/* Copyright (C) 2012 mbed.org, MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "Socket/Socket.h"
#include "Socket/Endpoint.h"
#include <cstring>

extern Serial pc;

using std::memset;

Endpoint::Endpoint()
{
    ESP8266 = ESP8266::getInstance();
    if (ESP8266 == NULL)
        error("Endpoint constructor error: no ESP8266 instance available!\r\n");
    reset_address();
}
Endpoint::~Endpoint() {}

void Endpoint::reset_address(void)
{
    _ipAddress[0] = '\0';
    _port = 0;
    _id = -1;
}

int Endpoint::set_address(const char* host, const int port)
{
    //Resolve DNS address or populate hard-coded IP address
    if(ESP8266->getHostByName(host, _ipAddress)) {
        _port = port;
        return 0;
    } else {
        return -1;
    }
}

char* Endpoint::get_address()
{
    return _ipAddress;
}

int   Endpoint::get_port()
{
    return _port;
}

int Endpoint::get_id()
{
    return _id;
}
