/*
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


#include "WizFi250Interface.h"

WizFi250Interface::WizFi250Interface(PinName tx, PinName rx, PinName cts, PinName rts, PinName reset, PinName alarm, int baud):
    WizFi250(tx, rx, cts, rts, reset, alarm, baud)
{

}

int WizFi250Interface::init(const char *name)
{
    return setAddress(name);
}

int WizFi250Interface::init(const char* ip, const char* netmask, const char* gateway, const char* name)
{
    return setAddress(ip, netmask, gateway, name);
}

int WizFi250Interface::connect(Security sec, const char* ssid, const char* phrase, WiFiMode mode)
{
    INFO("setSsid");
    setSsid(ssid);
    INFO("setSec");
    setSec(sec, phrase);

    switch (mode)
    {
    case WM_STATION:
        return join();
    case WM_AP:
        return 0;
    }
    WARN("end switch mode");
}

int WizFi250Interface::disconnect()
{

}

char* WizFi250Interface::getMACAddress()
{
    return _state.mac;
}

char* WizFi250Interface::getIPAddress()
{
    return _state.ip;
}

char* WizFi250Interface::getGateway()
{
    return _state.gateway;
}

//char* getNetworkMask();
