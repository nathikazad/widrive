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
/* Copyright (C) 2014 Wiznet, MIT License
 *  port to the Wiznet Module WizFi250
 */


#include "TCPSocketConnection.h"
#include <algorithm>

TCPSocketConnection::TCPSocketConnection() {}

int TCPSocketConnection::connect(const char* host, const int port)
{
    if (set_address(host, port) != 0)   return -1;
    INFO("TCPSocketConnection::connect: host: %s, port: %d", host, port);

    _server = false;
    _cid = _wizfi250->open(WizFi250::PROTO_TCP, get_address(), get_port());
    if (_cid < 0 )  return -1;

    return 0;
}

bool TCPSocketConnection::is_connected(void)
{
    bool _is_connected = _wizfi250->isConnected(_cid);
    if(!_is_connected)  _cid = -1;

    return _is_connected;
}

int TCPSocketConnection::send(char *data, int length)
{
    if (_cid < 0 || !is_connected())    return -1;

    return _wizfi250->send(_cid, data, length);
}

int TCPSocketConnection::send_all(char *data, int length)
{
    Timer tmr;
    int idx = 0;

    if(_cid <0 || !is_connected())  return -1;

    tmr.start();
    while((tmr.read_ms() < _timeout) || _blocking)
    {
        idx += _wizfi250->send(_cid, &data[idx], length - idx);
        if(idx < 0) return -1;

        if(idx == length)
            return idx;
    }
    return (idx == 0) ? -1 : idx;
}

int TCPSocketConnection::receive(char* data, int length)
{
    Timer tmr;
    int time = -1;

    if(_cid < 0 || !is_connected()) return -1;

    if(_blocking)
    {
        tmr.start();
        while(time < _timeout + 20)
        {
            if(_wizfi250->readable(_cid))
            {
                DBG("receive readable");
                break;
            }
            time = tmr.read_ms();
        }
        if(time >= _timeout + 20)
        {
            DBG("receive timeout");
            return 0;
        }
    }

    int nb_available = _wizfi250->recv(_cid, data, length);

    return nb_available;
}

int TCPSocketConnection::receive_all(char* data, int length)
{
    Timer tmr;
    int idx = 0;
    int time = -1;

    if(_cid < 0 || !is_connected()) return -1;

    tmr.start();

    while(time < _timeout || _blocking)
    {
        idx += _wizfi250->recv(_cid, &data[idx], length - idx);
        if (idx < 0)    return -1;

        if(idx == length)
            break;

        time = tmr.read_ms();
    }

    return (idx == 0) ? -1 : idx;
}

void TCPSocketConnection::acceptCID (int cid)
{
    char *ip;
    int port;
    _server = true;
    _cid = cid;


    if( _wizfi250->cmdSMGMT(cid) )  return;
    if( !_wizfi250->getRemote(_cid, &ip, &port))
    {
        set_address(ip, port);
    }
}
