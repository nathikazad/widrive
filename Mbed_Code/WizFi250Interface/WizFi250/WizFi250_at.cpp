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



void WizFi250::clearFlags()
{
    _state.ok = false;
    _state.failure = false;
    _state.res = RES_NULL;
    _state.n = 0;
}

int WizFi250::sendCommand(const char * cmd, const char *ACK, char *res , int timeout, int opt)
{
    char read;
    int i;
    Timer t;
    string checking;
    size_t found = string::npos;
    
    INFO("WizFi250_at:sendCommand ACK");

    //attach_rx(false);
    if (lockUart(timeout))  return -1;

    // flush the buffer
    while(_wizfi.readable())
    {
        printf("%c",_wizfi.getc());
    }

    clearFlags();

    INFO("command: '%s'", cmd)
    for (i=0; i< strlen(cmd); i++)
    {
        putUart(cmd[i]);
    }

    if(opt == 0)
    {

    }
    else if(opt == 1)
    {
        putUart('\r');
    }
    else
    {
        putUart('\r');
        putUart('\n');
    }
    unlockUart();

    while(1)
    {
        if(t.read_ms() > timeout)
        {
            while(_wizfi.readable())
                _wizfi.getc();

            DBG("check: %s\r\n", checking.c_str());
            return -1;
        }
        else if(_wizfi.readable())
        {
            read = _wizfi.getc();
            if(read != '\r' && read != '\n')
            {
                checking += read;
                found = checking.find(ACK);
                if (found != string::npos)
                {
                    wait(0.01);

                    while(_wizfi.readable())
                        _wizfi.getc();

                    break;
                }
            }
        }
    }
    DBG("check:%s\r\n", checking.c_str());

//    attach_rx(true);

    return 0;
}



int WizFi250::sendCommand(const char * cmd, Response res, int timeout, int opt)
{
    int i, cnt=0;
    Timer t;
    WARN("WizFi250: send command");
    INFO("command: %s", cmd);

  if (lockUart(timeout)) return -1;
  INFO("WizFi250: past lockUART");

    clearFlags();
    _state.res = res;

    for (i=0; i< strlen(cmd); i++)
    {
        putUart(cmd[i]);
    }

    if(opt == 0)
    {

    }
    else if(opt == 1)
    {
        putUart('\r');
    }
    else
    {
        putUart('\r');
        putUart('\n');
    }
    unlockUart();
    INFO("before timeout");

    if(timeout)
    {
        t.start();
        for(;;)
        {
            //INFO("looping: read_ms: %d, ok: %d, fail: %d, res: %d", t.read_ms(), _state.ok, _state.failure, _state.res);
            if (_state.ok && _state.res == RES_NULL){
                break;
            }
            if (_state.failure || t.read_ms() > timeout)
            {
                WARN("WizFi250_at: %d failure of timeout[%d]ms > %d\r\n",_state.failure, t.read_ms(), timeout);
                _state.res = RES_NULL;
                return -1;
            }
        }
        INFO("WizFi250_at: broken loop");

        t.stop();
    }
    _state.res = RES_NULL;

    return 0;
}

int WizFi250::cmdAT()
{
    int resp;

    resp = sendCommand("AT");

    DBG("%s",_state.dummyBuf);
    memset(_state.dummyBuf,0,sizeof(_state.dummyBuf));

    return resp;

    //return sendCommand("AT");
    //sendCommand("AT","[OK]");
}

int WizFi250::cmdMECHO(bool flg)
{
    int status;
    char cmd[CFG_CMD_SIZE];

    sprintf(cmd,"AT+MECHO=%d",flg);
    status = sendCommand(cmd);

    DBG("%s",_state.dbgRespBuf);
    return status;
}

int WizFi250::cmdUSET(int baud, char *flow)
{
    int status;
    char cmd[CFG_CMD_SIZE];

    sprintf(cmd,"AT+USET=%d,N,8,1,%s",baud, flow);
    status = sendCommand(cmd);

    if(status == 0)
    {
        wait(1);
        _state.buf->flush();
    }

    return status;
}

int WizFi250::cmdMMAC(const char *mac)
{
    int resp;
    char cmd[CFG_CMD_SIZE];
    const char xmac[] = "00:08:DC:1E:DE:D0";

    if (mac)
    {
        INFO("WizFi250_at::MMAC if");
        sprintf(cmd, "AT+MMAC=%s",mac);
        resp = sendCommand(cmd);
        INFO("WizFi250_at::MMAC end");
    }
    else
    {
        INFO("WizFi250_at::MMAC else");
        sprintf(cmd, "AT+MMAC=%s",xmac);
        resp = sendCommand(cmd);
        INFO("MMAC resp: %d\n", resp);
        if (!resp && strncmp(_state.mac, xmac, 17) == 0)
        {
            resp = -1;
        }
    }

    return resp;
}

int WizFi250::cmdWSET(WiFiMode mode, const char *ssid, const char *bssid, int channel)
{
    char cmd[CFG_CMD_SIZE];

    if(*bssid == NULL)
    {
        sprintf(cmd, "AT+WSET=%d,%s",mode, ssid);
    }
    else
    {
        sprintf(cmd, "AT+WSET=%d,%s,%s,%d",mode, ssid, bssid, channel);
    }

    return sendCommand(cmd);
}

int WizFi250::cmdWSEC(WiFiMode mode, const char *key, const char *sec)
{
    char cmd[CFG_CMD_SIZE];

    if(*sec == NULL)
    {
        sprintf(cmd, "AT+WSEC=%d,,%s",mode, key);
    }
    else
    {
        sprintf(cmd, "AT+WSEC=%d,%s,%s",mode, sec, key);
    }

    return sendCommand(cmd);
}

int WizFi250::cmdWJOIN()
{
    sendCommand("AT+WJOIN", RES_WJOIN, CFG_TIMEOUT);

    DBG("IP : %s",_state.ip);
    DBG("Gateway : %s",_state.gateway);

    return 0;
}

int WizFi250::cmdWLEAVE()
{
    sendCommand("AT+WLEAVE", RES_WJOIN, CFG_TIMEOUT);

    DBG("IP : %s",_state.ip);
    DBG("Gateway : %s",_state.gateway);

    return 0;
}


int WizFi250::cmdSCON ( const char *openType, const char *socketType, int localPort, const char *dataMode)
{
    char cmd[CFG_CMD_SIZE];
    sprintf(cmd,"AT+SCON=%s,%s,,,%d,%s",openType, socketType, localPort, dataMode);
    return sendCommand(cmd);
}

int WizFi250::cmdSCON ( const char *openType, const char *socketType, const char *remoteIp, int remotePort, int localPort, const char *dataMode)
{
    int resp;
    char cmd[CFG_CMD_SIZE];
    int local = 5000;

    if(localPort == NULL)
        sprintf(cmd,"AT+SCON=%s,%s,%s,%d,%s,%s",openType, socketType, remoteIp, remotePort, "", dataMode);
    else
        sprintf(cmd,"AT+SCON=%s,%s,%s,%d,%d,%s",openType, socketType, remoteIp, remotePort, localPort, dataMode);

    resp = sendCommand(cmd, RES_CONNECT, 5000 );
    DBG("Create CID : %s",_state.dummyBuf);

    return resp;
}

int WizFi250::cmdSSEND ( const char *data, int cid, int sendSize, const char *remoteIp, int remotePort, int Timeout )
{
    int i, resp;
    Timer t;
    char cmd[CFG_CMD_SIZE];


    if (lockUart(Timeout))    return -1;

    clearFlags();
    if(remotePort == NULL)
    {
        sprintf(cmd,"AT+SSEND=%d,%s,,%d",cid, remoteIp, sendSize);
    }
    else
    {
        sprintf(cmd,"AT+SSEND=%d,%s,%d,%d",cid, remoteIp, remotePort, sendSize);
    }

    _con[cid].send_length = sendSize;

    resp = sendCommand(cmd, RES_SSEND, Timeout, 1);
    unlockUart();
    if(resp){
        DBG("Fail cmdSSEND")
        return -1;
    }

    for(i=0; i<sendSize; i++)
    {
        putUart(data[i]);
    }
    unlockUart();

    if(Timeout)
    {
        t.start();
        for(;;)
        {
            if (_state.ok) break;
            if (_state.failure || t.read_ms() > Timeout)
            {
                WARN("failure or timeout\r\n");
                return -1;
            }
        }
        t.stop();
    }

    //INFO("data: '%s' size : %d\r\n",data, sendSize);

    return i;
}


int WizFi250::cmdCLOSE ( int cid )
{
    char cmd[CFG_CMD_SIZE];

    sprintf(cmd,"AT+SMGMT=%d",cid);
    return sendCommand(cmd);
}


int WizFi250::cmdFDNS (const char *host)
{
    char cmd[CFG_CMD_SIZE];
    int resp;

    sprintf(cmd,"AT+FDNS=%s,1000",host);
    resp = sendCommand(cmd, RES_FDNS, CFG_TIMEOUT);

    DBG("%s",_state.resolv);
    return resp;
}

int WizFi250::cmdSMGMT ( int cid )
{
    int resp;

    resp = sendCommand("AT+SMGMT=?", RES_SMGMT);
    return resp;
}
