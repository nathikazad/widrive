/*
 /* Copyright (C) 2013 gsfan, MIT License
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
/* Copyright (C) 2014 Wiznet, MIT License
 *  port to the Wiznet Module WizFi250
 */

#ifndef WIZFI250INTERFACE_H_
#define WIZFI250INTERFACE_H_

#include "WizFi250.h"

class WizFi250Interface : public WizFi250{
public:

    WizFi250Interface(PinName tx, PinName rx, PinName cts, PinName rts, PinName reset, PinName alarm = NC, int baud = 115200);

    int init(const char *name = NULL);
    int init(const char* ip, const char* mask, const char* gateway, const char* name = NULL);
    int connect(Security sec, const char* ssid, const char* phrase, WiFiMode mode = WM_STATION);
    int disconnect();
    char* getMACAddress();
    char* getIPAddress();
    char* getGateway();
    char* getNetworkMask();


};


#include "TCPSocketConnection.h"
#include "TCPSocketServer.h"

#include "Endpoint.h"
#include "UDPSocket.h"


#endif /* WIZFI250INTERFACE_H_ */
