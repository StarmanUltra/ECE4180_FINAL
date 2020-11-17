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

#include "mbed.h"
#include "ESP8266.h"
#include "Endpoint.h"
#include <string>
#include <algorithm>

//Debug is disabled by default
#if 0
#define DBG(x, ...)  printf("[ESP8266 : DBG]"x" \t[%s,%d]\r\n", ##__VA_ARGS__,__FILE__,__LINE__); 
#define WARN(x, ...) printf("[ESP8266 : WARN]"x" \t[%s,%d]\r\n", ##__VA_ARGS__,__FILE__,__LINE__); 
#define ERR(x, ...)  printf("[ESP8266 : ERR]"x" \t[%s,%d]\r\n", ##__VA_ARGS__,__FILE__,__LINE__); 
#define INFO(x, ...) printf("[ESP8266 : INFO]"x" \t[%s,%d]\r\n", ##__VA_ARGS__,__FILE__,__LINE__); 
#ifndef ESP8266_ECHO
#define ESP8266_ECHO 1
#endif
#else
#define DBG(x, ...)
#define WARN(x, ...)
#define ERR(x, ...)
#define INFO(x, ...)
#endif


ESP8266 *ESP8266::_inst;

ESP8266::ESP8266(PinName tx, PinName rx, PinName reset, int baud, int timeout) :
        _serial(tx, rx), _reset_pin(reset) {
    INFO("Initializing ESP8266 object");
    
    _baud = baud;
    _timeout = timeout;
    
    _serial.baud(_baud);
    
    _inst = this;
}

bool ESP8266::reset() {
    _reset_pin = 0;
    wait_us(20);
    _reset_pin = 1;
    
    // Send reboot command in case reset is not connected
    return command("\x03\r\n" "node.restart()") && execute();
}

bool ESP8266::init() {
    // Reset device to clear state
    return reset();
}

bool ESP8266::connect(const char *ssid, const char *phrase) {
    // Configure as station with passed ssid and passphrase
    if (!(command("wifi.setmode(wifi.STATION);") &&
          command("wifi.sta.config('") &&
          command(ssid) &&
          command("','") &&
          command(phrase) &&
          command("')") &&
          execute()))
        return false;
        
    // Wait for an IP address
    // TODO WISHLIST make this a seperate check so it can run asynch?
    Timer timer;
    timer.start();
    
    while (true) {
        int ip_len = 15;
        
        if (!(command("ip=wifi.sta.getip();") &&
              command("print(ip)") &&
              execute(_ip, &ip_len)))
            return false;
        
        _ip[ip_len] = 0;
        
        if (strcmp(_ip, "nil") != 0)
            return true;
        
        if (timer.read_ms() > _timeout)
            return false;
    }
}

bool ESP8266::disconnect() {
    int ip_len = 15;
    
    if (!(command("wifi.sta.disconnect();") &&
          command("ip=wifi.sta.getip();") &&
          command("print(ip)") &&
          execute(_ip, &ip_len)))
        return false;
        
    _ip[ip_len] = 0;
    
    return (strcmp(_ip, "nil") == 0);
}

bool ESP8266::is_connected() {
    return (strcmp(_ip, "nil") != 0);
}

bool ESP8266::open(bool type, char* ip, int port, int id) {
    // Create the actual connection
    if (!(command("c=net.createConnection(") &&
          command(type ? "net.TCP" : "net.UDP") &&
          command(")") &&
          execute()))
        return false;
        
    // Setup handlers for connecting and disconnecting
    if (!(command("cc=nil;") &&
          command("c:on('connection',function() cc=true end);") &&
          command("c:on('disconnection',function() cc=false end)") &&
          execute()))
        return false;
    
    // Setup functions for sending and recieving characters on the esp side
    if (!(command("cm='';") &&
          command("function cs(n) c:send(n) end;") &&
          command("function ca() print(#cm) end;") &&
          command("function cr(n) " 
                    "d=cm:sub(1,n):gsub('.', function(s) "
                      "return s.format('\\\\%03d', s:byte(1)) "
                    "end);"
                    "cm=cm:sub(n+1,-1);" 
                    "print(d) "
                  "end;") && 
          command("c:on('receive',function(c,n) cm=cm..n end)") && 
          execute()))
        return false;
    
    // Convert port to a string    
    char port_buf[16];
    sprintf(port_buf, "%d", port);
        
    // Connect to the ip address
    if (!(command("c:connect(") &&
          command(port_buf) &&
          command(",'") &&
          command(ip) &&
          command("')") &&
          execute()))
        return false;
        
    // Wait for it to connect
    // TODO WISHLIST make this a seperate check so it can run asynch?
    Timer timer;
    timer.start();
    
    while (true) {
        char con_buf[5];
        int con_len = 5;
        
        if (!(command("print(cc)") && execute(con_buf, &con_len)))
            return false;
        
        if (memcmp(con_buf, "true", con_len) == 0)
            return true;
        else if (memcmp(con_buf, "false", con_len) == 0)
            return false;
        
        if (timer.read_ms() > _timeout)
            return false;
    }
}
    
bool ESP8266::close() {
    return command("c:close();" "c=nil") && execute();
}

bool ESP8266::send(const char *buffer, int len) {     
    for (int line = 0; line < len; line += ESP_MAX_LINE) {
        if (!command("cs('"))
            return false;

        for (int i = 0; i < ESP_MAX_LINE && line+i < len; i++) {
            int a = buffer[line+i] / 100;
            int b = (buffer[line+i] - a*100) / 10;
            int c = (buffer[line+i] - a*100 - b*10);
            
            if (serialputc('\\') < 0 ||
                serialputc(a + '0') < 0 ||
                serialputc(b + '0') < 0 ||
                serialputc(c + '0') < 0)
                return false;
        }
        
        if (!(command("')") && execute()))
            return false;
    }
    
    return true;
}

bool ESP8266::recv(char *buffer, int *len) {
    char len_buf[16];
    sprintf(len_buf, "%d", *len);
    
    if (!(command("cr(") &&
          command(len_buf) &&
          command(")")))
        return false;
        
   if (!(command("\r\n") && discardEcho()))
        return false;
        
    // Read in response
    for (int i = 0; i < *len; i++) {
        int e = serialgetc();
        
        if (e == '\r') {
            *len = i;
            break;
        } else if (e != '\\') {
            return false;
        }
        
        int a = serialgetc();
        int b = serialgetc();
        int c = serialgetc();
        
        if (a < 0 || b < 0 || c < 0)
            return false;
            
        buffer[i] = (a-'0')*100 + (b-'0')*10 + (c-'0');
    }
    
    // Flush to next prompt
    return flush();
}

int ESP8266::putc(char c) {
    char buffer[1] = { c };
    
    return send(buffer, 1) ? 0 : -1;
}

int ESP8266::getc() {
    char buffer[1];
    
    while (true) {
        int len = 1;
        
        if (!recv(buffer, &len))
            return -1;
            
        if (len > 0)
            break;
    }
    
    return buffer[0];
}

int ESP8266::writeable() {
    // Always succeeds since message can be temporarily stored on the esp
    return 1;
}

int ESP8266::readable() {
    char count_buf[16];
    int count_len = 15;
    
    if (!(command("ca()") && execute(count_buf, &count_len)))
        return false;
        
    count_buf[count_len] = 0;
    return atoi(count_buf);
}   

const char *ESP8266::getIPAddress() {
    if (strcmp(_ip, "nil") != 0)
        return _ip;
    else
        return 0;
}

bool ESP8266::getHostByName(const char *host, char *ip) {
    if (!(command("dh='") && 
          command(host) &&
          command("';") &&
          command("dn=nil;") &&
          command("if dh:match('%d+.%d+.%d+.%d+') then ") &&
          command("dn=dh ") &&
          command("else ") &&
          command("dc=net.createConnection(net.TCP);") &&
          command("dc:dns(dh,function(dc,ip) dn=ip end);") &&
          command("dc=nil ") &&
          command("end") &&
          execute()))
        return false;

    // Wait for a response
    Timer timer;
    timer.start();
    
    while (true) {
        int ip_len = 15;
        
        if (!(command("print(dn)") && execute(ip, &ip_len)))
            return false;
            
        ip[ip_len] = 0;
            
        if (strcmp(ip, "nil") != 0)
            return true;
        
        if (timer.read_ms() > _timeout)
            return false;
    }
}

int ESP8266::serialgetc() {
    Timer timer;
    timer.start();
    
    while (true) {
        if (_serial.readable()) {
            char c = _serial.getc();
#ifdef ESP8266_ECHO
            printf("%c", c);
#endif
            return c;
        }
            
        if (timer.read_ms() > _timeout)
            return -1;
    }
}

int ESP8266::serialputc(char c) {
    Timer timer;
    timer.start();
    
    while (true) {
        if (_serial.writeable())
            return _serial.putc(c);
            
        if (timer.read_ms() > _timeout)
            return -1;
    }
}

bool ESP8266::discardEcho() {
    while (true) {
        int c = serialgetc();
        
        if (c < 0)
            return false;
        else if (c == '\n')
            return true;
    }
}

bool ESP8266::flush() {
    while (true) {
        int c = serialgetc();
        
        if (c < 0)
            return false;
        else if (c == '>')
            return true;
    }
}

bool ESP8266::command(const char *cmd) {
    DBG("command sent:\t %s", cmd);
    
    for (int i = 0; cmd[i]; i++) {
        if (serialputc(cmd[i]) < 0)
            return false;
    }
    
    return true;
} 

bool ESP8266::execute(char *resp_buf, int *resp_len) {
    // Finish command with a newline
    if (!(command("\r\n") && discardEcho()))
        return false;
        
    // Read in response if any
    if (resp_buf && resp_len) {
        int i;
        
        for (i = 0; i < *resp_len; i++) {
            int c = serialgetc();
                
            if (c < 0)
                return false;
            
            if (c == '\r') {
                *resp_len = i;
                break;
            }
            
            resp_buf[i] = c;
        }
        
        DBG("command response:\t %.*s", *resp_len, resp_buf);
    }
    
    return flush();
}
