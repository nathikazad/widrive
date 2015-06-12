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
 
#include "Socket.h"
#include <cstring>

Socket::Socket() {
    _wizfi250 = WizFi250::getInstance();
    if (_wizfi250 == NULL)
        error("Socket constructor error: no WizFi250 instance available!\r\n");
    _blocking = true;
    _timeout = 1500;

    _server = false;
    _cid = -1;
    _port = 0;
}

void Socket::set_blocking(bool blocking, unsigned int timeout) {
    _blocking = blocking;
    _timeout = timeout;
}

int Socket::close() {
    if (_cid < 0) return 0;

    _wizfi250->close(_cid);
    _cid = -1;
    return 0;
}

Socket::~Socket() {
    close(); //Don't want to leak
}
