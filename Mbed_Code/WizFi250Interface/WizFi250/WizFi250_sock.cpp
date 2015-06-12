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


int WizFi250::getHostByName(const char * host, char *ip)
{
    INFO("WizFi250_sock::get host by name: host: %s, ip: %s", host, ip);
    int i, flg = 0;

    if(!isAssociated() || _state.status != STAT_READY)  return -1;

    for(i=0; i<strlen(host); i++)
    {
        if( (host[i] < '0' || host[i] > '9') && host[i] != '.')
        {
            flg = 1;
            break;
        }
    }
    if (!flg)
    {
        strncpy(ip, host, 16);
        return 0;
    }

    if ( cmdFDNS(host) )
    {
        wait_ms(1000);
        if( cmdFDNS(host) ) return -1;
    }
    strncpy(ip, _state.resolv, 16);
    return 0;
}

int WizFi250::open(Protocol proto, const char *ip, int remotePort, int localPort, void(*func)(int))
{
    INFO("WizFi250_sock::open: ip: %s, remoteport: %d, localport: %d", ip, remotePort, localPort);
    int cid;

    if (!isAssociated() || _state.status != STAT_READY)     return -1;

    _state.cid = -1;

    if (proto == PROTO_TCP)
    {
        if( cmdSCON( "O","TCN",ip, remotePort, localPort, "0" ) )   return -1;
    }
    else if(proto == PROTO_UDP)
    {
        if( cmdSCON( "O","UCN",ip, remotePort, localPort, "0" ) )   return -1;
    }
    if(_state.cid < 0) return -1;
    cid = _state.cid;
    _con[cid].protocol = proto;
    _con[cid].type = TYPE_CLIENT;
    _con[cid].func = func;
    return cid;
}

int WizFi250::listen (Protocol proto, int port, void(*func)(int))
{
    int cid;

    if(!isAssociated() || _state.status != STAT_READY)  return -1;

    _state.cid = -1;
    
    INFO("WizFi250_sock");

    if(proto == PROTO_TCP)
    {
        if( sendCommand("AT+MEVTMSG=1") )   return -1;
        if( cmdSCON("O","TSN",port) )       return -1;
    }
    else
    {
        if( cmdSCON("O","USN",port) )   return -1;
    }

    if (_state.cid < 0) return -1;
    cid = _state.cid;
    _con[cid].protocol = proto;
    _con[cid].type = TYPE_SERVER;
    _con[cid].func = func;

    return cid;
}

int WizFi250::close (int cid)
{
    if(!isConnected(cid))   return -1;

    _con[cid].connected = false;
    return cmdCLOSE(cid);
}


void WizFi250::initCon ( int cid, bool connected )
{
    _con[cid].parent = -1;      // It will be delete because It is not need
    _con[cid].func = NULL;
    _con[cid].accept = false;

//#ifndef CFG_ENABLE_RTOS
    if ( _con[cid].buf == NULL )
    {
        _con[cid].buf = new CircBuffer<char>(CFG_DATA_SIZE);
        if ( _con[cid].buf == NULL )    error("Can't allocate memory");
    }
//#endif
    if ( _con[cid].buf != NULL )
    {
        _con[cid].buf->flush();
    }
    _con[cid].connected = connected;
}

int WizFi250::send(int cid, const char *buf, int len)
{
    if(!isConnected(cid)) return -1;

    if((_con[cid].protocol == PROTO_TCP) ||
            (_con[cid].protocol == PROTO_UDP && _con[cid].type == TYPE_CLIENT) )
    {
//        if ( len > CFG_DATA_SIZE)   len = CFG_DATA_SIZE;
        return cmdSSEND(buf,cid,len);
    }
    else
    {
        return -1;
    }
}

int WizFi250::sendto (int cid, const char *buf, int len, const char *ip, int port)
{
    if(!isConnected(cid))   return -1;

    if((_con[cid].protocol == PROTO_UDP && _con[cid].type == TYPE_SERVER))
    {
        if ( len > CFG_DATA_SIZE )  len = CFG_DATA_SIZE;
        return cmdSSEND(buf,cid,len,ip,port);
    }
    else
    {
        return -1;
    }
}

int WizFi250::recv (int cid, char *buf, int len)
{
    int i;

    if (!isConnected(cid))  return -1;

    if (_con[cid].buf == NULL ) return 0;
    while (!_con[cid].received && _state.mode != MODE_COMMAND);
    _con[cid].received = false;
    for(i=0; i<len; i++)
    {
        if(_con[cid].buf->dequeue(&buf[i]) == false)    break;
    }
    setRts(true);       // release
    return i;
}

int WizFi250::recvfrom (int cid, char *buf, int len, char *ip, int *port)
{
    int i;

    if (!isConnected(cid))  return -1;

    if (_con[cid].buf == NULL)  return 0;

    while (!_con[cid].received && _state.mode != MODE_COMMAND);

    _con[cid].received = false;
    for(i=0; i<len; i++)
    {
        if( _con[cid].buf->dequeue(&buf[i]) == false )  break;
    }
    //buf[i] = '\0';
    strncpy(ip, _con[cid].ip, 16);
    *port = _con[cid].port;
    setRts(true);       // release

    return i;
}

int WizFi250::readable (int cid)
{
    if (!isConnected(cid))  return -1;

    if(_con[cid].buf == NULL)   return -1;
    return _con[cid].buf->available();
}

bool WizFi250::isConnected (int cid)
{
    if ( cid < 0 || cid >=8 ) return false;

    return _con[cid].connected;
}

int WizFi250::accept (int cid)
{
    if(!isConnected(cid))   return -1;

    if(_con[cid].connected && _con[cid].accept)
    {
        _con[cid].accept = false;
        return cid;
    }

    return -1;
}

int WizFi250::getRemote(int cid, char **ip, int *port)
{
    if (!isConnected(cid))  return -1;

    *ip = _con[cid].ip;
    *port = _con[cid].port;
    return 0;
}
