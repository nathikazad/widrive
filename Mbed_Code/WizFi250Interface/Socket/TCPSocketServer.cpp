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

#include "TCPSocketServer.h"
#include <string>

TCPSocketServer::TCPSocketServer() {}

// Server initialization
int TCPSocketServer::bind(int port) {

    _port = port;
    return 0;
}

int TCPSocketServer::listen(int backlog) {
    _server = true;
    if (_cid < 0) {
        // Socket open
        _server = false;
        _cid = _wizfi250->listen(WizFi250::PROTO_TCP, _port);

        DBG("TEST CID : %d",_cid);
        if (_cid < 0) return -1;
    }

    if (backlog != 1)
        return -1;
    return 0;
}


int TCPSocketServer::accept(TCPSocketConnection& connection) {
    int acid = -1;

    while (1) {
        while (acid < 0) {
            acid = _wizfi250->accept(_cid);
        }

        if (acid >= 0) {
            printf("TEST acid : %d\r\n",acid);
            connection.acceptCID(acid);
            return 0;
        }
    }
    return -1;
}
