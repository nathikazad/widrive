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

#include "WizFi250.h"

void WizFi250::setReset(bool flg)
{
    if( flg )
    {
        // low
        _reset.output();
        _reset = 0;
    }
    else
    {
        // high z
        _reset.input();
        _reset.mode(PullNone);
    }
}

void WizFi250::isrUart()
{
    char c;

    c = getUart();
    //_debug.putc(c);
    recvData(c);
}

int WizFi250::getUart()
{
#ifdef CFG_UART_DIRECT
    return _uart->RBR;
#else
    return _wizfi.getc();
#endif
}

void WizFi250::putUart (char c)
{
#ifdef CFG_UART_DIRECT
    while(!(_uart->LSR & (1<<5)));
    _uart->THR = c;
#else
    _wizfi.putc(c);
#endif
}

void WizFi250::setRts (bool flg)
{
    if (flg)
    {
        if(_flow == 2)
        {
            if(_rts)
            {
                _rts->write(0); // low
            }
        }
    }
    else
    {
        if(_flow == 2)
        {
            if(_rts)
            {
                _rts->write(1); // high
            }
        }
    }
}

int WizFi250::lockUart (int ms)
{
    Timer t;
    
    INFO("WizFi250_hal::lockUart");

    if(_state.mode != MODE_COMMAND)
    {
        t.start();
        while(_state.mode != MODE_COMMAND)
        {
            if(t.read_ms() >= ms)
            {
                WARN("lock timeout (%d)\r\n", _state.mode);
                return -1;
            }
        }
    }

#ifdef CFG_ENABLE_RTOS
    if (_mutexUart.lock(ms) != osOK) return -1;
#endif

    if(_flow == 2)
    {
        if(_cts && _cts->read())
        {
            // CTS check
            t.start();
            while (_cts->read())
            {
                if(t.read_ms() >= ms)
                {
                    DBG("cts timeout\r\n");
                    return -1;
                }
            }
        }
    }

    setRts(false);      // blcok
    return 0;
}

void WizFi250::unlockUart()
{
    setRts(true);       // release
#ifdef CFG_ENABLE_RTOS
    _mutexUart.unlock();
#endif
}

void WizFi250::initUart (PinName cts, PinName rts, PinName alarm, int baud)
{
    _baud = baud;
    if (_baud) _wizfi.baud(_baud);

    _wizfi.attach(this, &WizFi250::isrUart, Serial::RxIrq);

    _cts = NULL;
    _rts = NULL;
    _flow = 0;

#if defined(TARGET_FRDM_KL25Z)
    _uart = UART1;
#endif

    if(cts != NC)
    {
        _cts = new DigitalIn(cts);
    }
    if(rts != NC)
    {
        _rts = new DigitalOut(rts);
        _flow = 2;
    }
}
